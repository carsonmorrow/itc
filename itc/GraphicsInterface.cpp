#include "GraphicsInterface.h"
#include "com.h"
#include "memcheck.h"

GraphicsInterface::GraphicsInterface()
{
	InitializeCriticalSection(&_csCreateDisplay);
	_hTimerQueue = CreateTimerQueue();
	_hDisplayTimer = NULL;
	_nDisplayType = 0;
	_override_always_up = false;
	_bmInfo = NULL;

	if (m_ssSettings.bOSDAllMonitors)
	{
		CMonitors cm;

		for (int i = 0; i < cm.GetCount(); ++i)
		{
			_ges.push_back(new GraphicsEngine(i));
		}
	}
	else
	{
		GraphicsEngine* ge = new GraphicsEngine(m_ssSettings.nOSDMonitor);
		_ges.push_back(ge);
	}
}

GraphicsInterface::~GraphicsInterface()
{
	DeleteTimerQueueEx(_hTimerQueue, INVALID_HANDLE_VALUE);
	DeleteCriticalSection(&_csCreateDisplay);

	for (const auto& ge : _ges)
	{
		delete ge;
	}
}

void GraphicsInterface::Replace(wstring & wstrText, wstring wstrFind, wstring wstrReplace)
{
	size_t pos = 0;

	pos = wstrText.find(wstrFind);
	while (pos != wstring::npos)
	{
		wstrText.replace(pos, wstrFind.length(), wstrReplace);
		pos = wstrText.find(wstrFind);
	}

	return;
}

bool GraphicsInterface::IsDisplayVisible()
{
	return _ges.at(0)->IsDisplayVisible();
}

void GraphicsInterface::DisplayShow()
{
	for (const auto& ge : _ges)
	{
		ge->DisplayShow(m_ssSettings.bFadeEffects);
	}

	return;
}

void GraphicsInterface::DisplayHide()
{
	for (const auto& ge : _ges)
	{
		ge->DisplayHide(m_ssSettings.bFadeEffects);
	}

	return;
}

void GraphicsInterface::DisplayInfo(wstring wstrText, bool bArtwork, wchar_t *wcArt, int display_time, bool bUpdateOnly)
{
	GraphicsEngineOptions geo;
	RECT rcScreen;
	CMonitors cm;
	POINT pt = { 0, 0 };
	Color cNull = Color(0, 0, 0, 0);
	Color cTemp;
	Color cOutline;
	Color cArtOutline;
	Color cDropShadow;
	REAL rTemp;

	if (!m_ssSettings.bVisualFeedback)
		return;

	if (m_ssSettings.bNoDisplayWhenActive && m_ITCom->IsActiveWindow())
		return;

	if (bUpdateOnly && (_nDisplayType == 1 || !IsDisplayVisible()))
		return;

	EnterCriticalSection(&_csCreateDisplay);
	_nDisplayType = 0;

	// Configure
	geo.nDisplayAlpha = m_ssSettings.nOSDAlpha;
	geo.bDisplayTopMost = m_ssSettings.bOSDTopMost;
	geo.nMaxWidth = m_ssSettings.nOSDMaxWidth;
	geo.nArtScaleMode = m_ssSettings.nArtMode == 3 ? ART_SCALEMODEHEIGHT : ART_SCALEMODESIZE;
	geo.saAlign = (Gdiplus::StringAlignment)m_ssSettings.nOSDTextJustify;
	geo.bForceToWidth = m_ssSettings.bOSDForceWidth;

	if (!bArtwork || NULL == wcArt || 0 == wcscmp(wcArt, L""))
	{
		for (const auto& ge : _ges)
		{
			ge->SetOptions(geo);
			ge->BeginRender(5);
		}
	}
	else
	{
		for (const auto& ge : _ges)
		{
			ge->SetOptions(geo);
			ge->BeginRender(m_ssSettings.nDisplayLayout);
		}
	}

	// Set background and borders
	rTemp = (REAL)m_ssSettings.nOSDBorderSize;
	cOutline = Color(GetRValue(m_ssSettings.lOSDBorderColor), GetGValue(m_ssSettings.lOSDBorderColor), GetBValue(m_ssSettings.lOSDBorderColor));
	cTemp = Color(GetRValue(m_ssSettings.lOSDBkColorRef), GetGValue(m_ssSettings.lOSDBkColorRef), GetBValue(m_ssSettings.lOSDBkColorRef));
	cDropShadow = Color(GetRValue(m_ssSettings.lDropShadowColor), GetGValue(m_ssSettings.lDropShadowColor), GetBValue(m_ssSettings.lDropShadowColor));
	cArtOutline = Color(GetRValue(m_ssSettings.lArtBorderColor), GetGValue(m_ssSettings.lArtBorderColor), GetBValue(m_ssSettings.lArtBorderColor));
	for (const auto& ge : _ges)
	{
		ge->SetDisplayBorder(m_ssSettings.bOSDOutlineMode ? cNull : cOutline, rTemp);
		ge->SetDisplayBackground(m_ssSettings.bOSDOutlineMode ? cNull : cTemp);
	}

		// Add album art if needed
		if (bArtwork && m_ssSettings.bShowArtwork)
		{
			for (const auto& ge : _ges)
			{
				ge->AddArt(wcArt, false, cArtOutline, 1.5f, (float)m_ssSettings.nArtConstSize, m_ssSettings.nArtMode == 3 ? IMG_SCALENONE : IMG_SCALEALL);
			}
		}

	// Cache text height for shapes
	static REAL text_height;
	if (!bUpdateOnly)
		text_height = _ges.at(0)->GetTextHeight(m_ssSettings.wcFontFace, (float)m_ssSettings.nOSDFontPoint, m_ssSettings.nOSDFontStyle);

	// Break string into lines, add to display
	wchar_t *lpToken = NULL;
	wchar_t *lpContext = NULL;
	std::wstring line;
	size_t current_position = 0;
	size_t next_rating = 0;
	size_t next_format = 0;
	int old_font_style = m_ssSettings.nOSDFontStyle;
	cTemp = Color(GetRValue(m_ssSettings.lOSDColorRef), GetGValue(m_ssSettings.lOSDColorRef), GetBValue(m_ssSettings.lOSDColorRef));
	Color cShapeFill = Color(GetRValue(m_ssSettings.lRatingShapeFill), GetGValue(m_ssSettings.lRatingShapeFill), GetBValue(m_ssSettings.lRatingShapeFill));
	Color cShapeOutline = Color(GetRValue(m_ssSettings.lRatingShapeOutline), GetGValue(m_ssSettings.lRatingShapeOutline), GetBValue(m_ssSettings.lRatingShapeOutline));
	lpToken = wcstok_s(const_cast<wchar_t*>(wstrText.c_str()), L"\n", &lpContext);

	while (NULL != lpToken && m_ssSettings.nDisplayLayout != 4)
	{
		for (const auto& ge : _ges)
		{
			ge->BeginLine();
		}

			line = lpToken;
		current_position = 0;
		do
		{
			next_rating = line.find(L"%rating%", current_position);
			next_format = line.find(L"<", current_position);

			if ((next_rating == std::wstring::npos && next_format != std::wstring::npos) ||
				(next_format != std::wstring::npos && next_rating != std::wstring::npos &&
					next_format < next_rating))
			{
				// format
				if ((next_format - current_position) > 0)
				{
					for (const auto& ge : _ges)
					{
						ge->AddString(line.substr(current_position, next_format - current_position).c_str(), false,
							m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint, cTemp, m_ssSettings.wcFontFace,
							m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f, m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f,
							cDropShadow);
					}
				}
				if (line.compare(next_format, 3, L"<b>") == 0)
				{
					m_ssSettings.nOSDFontStyle |= FontStyleBold;
					current_position = next_format + 3;
				}
				else if (line.compare(next_format, 3, L"<i>") == 0)
				{
					m_ssSettings.nOSDFontStyle |= FontStyleItalic;
					current_position = next_format + 3;
				}
				else if (line.compare(next_format, 4, L"</b>") == 0)
				{
					m_ssSettings.nOSDFontStyle &= ~FontStyleBold;
					current_position = next_format + 4;
				}
				else if (line.compare(next_format, 4, L"</i>") == 0)
				{
					m_ssSettings.nOSDFontStyle &= ~FontStyleItalic;
					current_position = next_format + 4;
				}
				else
				{
					for (const auto& ge : _ges)
					{
						ge->AddString(line.substr(next_format, 1).c_str(), false,
							m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint, cTemp, m_ssSettings.wcFontFace,
							m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f, m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f,
							cDropShadow);
					}
					current_position = next_format + 1;
				}
			}
			else if ((next_format == std::wstring::npos && next_rating != std::wstring::npos) ||
				(next_format != std::wstring::npos && next_rating != std::wstring::npos &&
					next_rating < next_format))
			{
				// rating
				if ((next_rating - current_position) > 0)
				{
					for (const auto& ge : _ges)
					{
						ge->AddString(line.substr(current_position, next_rating - current_position).c_str(), false,
							m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint, cTemp, m_ssSettings.wcFontFace,
							m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f, m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f,
							cDropShadow);
					}
				}

				// Add rating
				long lRating = m_ITCom->GetTrackRating();
				long lWhole = (lRating - (lRating % 20)) / 20;
				lRating = lRating - lWhole * 20;
				long lHalf = (lRating - (lRating % 10)) / 10;
				long lEmpty = 5 - lWhole - lHalf;

				RectF rfShape = RectF(0, 0, text_height, text_height);
				for (int i = 0; i < lWhole; i++)
				{
					for (const auto& ge : _ges)
					{
						if (m_ssSettings.nRatingType == 1)
							ge->AddImage(m_ssSettings.wcRatingFullPath);
						else
							ge->AddShape((ShapeType)m_ssSettings.nRatingShapeType, FillAll, rfShape, false, cShapeOutline, 3.0, cShapeFill);
					}
				}
				for (int i = 0; i < lHalf; i++)
				{
					for (const auto& ge : _ges)
					{
						if (m_ssSettings.nRatingType == 1)
							ge->AddImage(m_ssSettings.wcRatingHalfPath);
						else
							ge->AddShape((ShapeType)m_ssSettings.nRatingShapeType, (ShapeStyle)(m_ssSettings.nRatingShapeStyle + 2), rfShape, false, cShapeOutline, 3.0, cShapeFill);
					}
				}
				for (int i = 0; i < lEmpty; i++)
				{
					for (const auto& ge : _ges)
					{
						if (m_ssSettings.nRatingType == 1)
							ge->AddImage(m_ssSettings.wcRatingEmptyPath);
						else
							ge->AddShape((ShapeType)m_ssSettings.nRatingShapeType, FillNone, rfShape, false, cShapeOutline, 3.0, cShapeFill);
					}
				}

				current_position = next_rating + 8;
			}
			else
			{
				// no special processing needed
				if (line.substr(current_position, std::wstring::npos).length() > 0)
				{
					for (const auto& ge : _ges)
					{
						ge->AddString(line.substr(current_position, std::wstring::npos).c_str(), false,
							m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint, cTemp, m_ssSettings.wcFontFace,
							m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f, m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f,
							cDropShadow);
					}
				}
			}
		} while (next_format != std::wstring::npos || next_rating != std::wstring::npos);

		for (const auto& ge : _ges)
		{
			ge->EndLine();
		}

		lpToken = wcstok_s(NULL, L"\n", &lpContext);
	}

	// Restore font style
	m_ssSettings.nOSDFontStyle = old_font_style;

	// Show display, start timer
	if (_bmInfo)
	{
		delete _bmInfo;
		_bmInfo = NULL;
	}

	if (m_ssSettings.bOSDAllMonitors)
	{
		bool has_track_pos = (NULL != wcsstr(m_ssSettings.wcOSDFormat, L"%track_position%"));
		int monitor = 1;

		for (const auto& ge : _ges)
		{
			// Position display
			cm.GetMonitorRect(monitor, rcScreen);

			pt.x = rcScreen.left + (LONG)ceilf((m_ssSettings.nOSDHoriz / 100.0f) * (rcScreen.right - rcScreen.left));
			pt.y = rcScreen.top + (LONG)ceilf((m_ssSettings.nOSDVert / 100.0f) * (rcScreen.bottom - rcScreen.top));

			if (has_track_pos)
				ge->EndRender(pt, rcScreen);
			else
				ge->EndRender(pt, rcScreen, m_ssSettings.bOSDAlwaysUp, &_bmInfo);

			++monitor;
		}
	}
	else
	{
		// Position display
		if (!cm.GetMonitorRect(m_ssSettings.nOSDMonitor, rcScreen))
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

		pt.x = rcScreen.left + (LONG)ceilf((m_ssSettings.nOSDHoriz / 100.0f) * (rcScreen.right - rcScreen.left));
		pt.y = rcScreen.top + (LONG)ceilf((m_ssSettings.nOSDVert / 100.0f) * (rcScreen.bottom - rcScreen.top));

		if (NULL != wcsstr(m_ssSettings.wcOSDFormat, L"%track_position%"))
		{
			for (const auto& ge : _ges)
			{
				ge->EndRender(pt, rcScreen);
			}
		}
		else
		{
			for (const auto& ge : _ges)
			{
				ge->EndRender(pt, rcScreen, m_ssSettings.bOSDAlwaysUp, &_bmInfo);
			}
		}
	}

	// Display should be shown for xx seconds, then hidden
	if (!bUpdateOnly)
	{
		if (!m_ssSettings.bOSDAlwaysUp)
		{
			if (NULL != _hDisplayTimer)
				DeleteTimerQueueTimer(_hTimerQueue, _hDisplayTimer, INVALID_HANDLE_VALUE);
			_hDisplayTimer = NULL;
			CreateTimerQueueTimer(&_hDisplayTimer, _hTimerQueue, DisplayTimerCallback, (PVOID)this, display_time, 0, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
		}


		DisplayShow();
	}

	LeaveCriticalSection(&_csCreateDisplay);

	return;
}

void GraphicsInterface::DisplayStatus(wstring wstrText, int display_time)
{
	GraphicsEngineOptions geo;
	RECT rcScreen;
	CMonitors cm;
	POINT pt = { 0, 0 };
	Color cNull = Color(0, 0, 0, 0);
	Color cTemp;
	Color cOutline;
	Color cDropShadow;
	REAL rTemp;

	if (!m_ssSettings.bVisualFeedback)
		return;

	if (m_ssSettings.bNoDisplayWhenActive && m_ITCom->IsActiveWindow())
		return;

	EnterCriticalSection(&_csCreateDisplay);
	_nDisplayType = 1;

	// Configure
	geo.nDisplayAlpha = m_ssSettings.nOSDAlpha;
	geo.bDisplayTopMost = m_ssSettings.bOSDTopMost;
	geo.nMaxWidth = m_ssSettings.nOSDMaxWidth;
	geo.nArtScaleMode = m_ssSettings.nArtMode == 3 ? ART_SCALEMODEHEIGHT : ART_SCALEMODESIZE;
	geo.saAlign = (Gdiplus::StringAlignment)m_ssSettings.nOSDTextJustify;
	geo.bForceToWidth = m_ssSettings.bOSDForceWidth;
	for (const auto& ge : _ges)
	{
		ge->SetOptions(geo);
		// Begin render process
		ge->BeginRender(5);
	}

		// Set background and borders
		rTemp = (REAL)m_ssSettings.nOSDBorderSize;
	cOutline = Color(GetRValue(m_ssSettings.lOSDBorderColor), GetGValue(m_ssSettings.lOSDBorderColor), GetBValue(m_ssSettings.lOSDBorderColor));
	cTemp = Color(GetRValue(m_ssSettings.lOSDBkColorRef), GetGValue(m_ssSettings.lOSDBkColorRef), GetBValue(m_ssSettings.lOSDBkColorRef));
	cDropShadow = Color(GetRValue(m_ssSettings.lDropShadowColor), GetGValue(m_ssSettings.lDropShadowColor), GetBValue(m_ssSettings.lDropShadowColor));
	for (const auto& ge : _ges)
	{
		ge->SetDisplayBorder(m_ssSettings.bOSDOutlineMode ? cNull : cOutline, rTemp);
		ge->SetDisplayBackground(m_ssSettings.bOSDOutlineMode ? cNull : cTemp);
	}

		// Cache text height for shapes
		REAL text_height = _ges.at(0)->GetTextHeight(m_ssSettings.wcFontFace, (float)m_ssSettings.nOSDFontPoint, m_ssSettings.nOSDFontStyle);

	// Break string into lines, add to display
	wchar_t *lpToken = NULL;
	wchar_t *lpContext = NULL;
	wchar_t *lpRating = NULL;
	wchar_t *lpVolume = NULL;
	wchar_t *lpBeforeRating = NULL;
	cTemp = Color(GetRValue(m_ssSettings.lOSDColorRef), GetGValue(m_ssSettings.lOSDColorRef), GetBValue(m_ssSettings.lOSDColorRef));
	Color cShapeFill = Color(GetRValue(m_ssSettings.lRatingShapeFill), GetGValue(m_ssSettings.lRatingShapeFill), GetBValue(m_ssSettings.lRatingShapeFill));
	Color cShapeOutline = Color(GetRValue(m_ssSettings.lRatingShapeOutline), GetGValue(m_ssSettings.lRatingShapeOutline), GetBValue(m_ssSettings.lRatingShapeOutline));
	lpToken = wcstok_s(const_cast<wchar_t*>(wstrText.c_str()), L"\n", &lpContext);
	while (NULL != lpToken && m_ssSettings.nDisplayLayout != 4)
	{
		for (const auto& ge : _ges)
		{
			ge->BeginLine();
		}
			// check for %rating% in string, replace with img rating
			lpRating = wcsstr(lpToken, L"%rating%");
		lpVolume = wcsstr(lpToken, L"%volume%");
		if (NULL != lpVolume)
		{
			// Break up line
			lpBeforeRating = new wchar_t[lpVolume - lpToken + 1];
			wcsncpy_s(lpBeforeRating, lpVolume - lpToken + 1, lpToken, lpVolume - lpToken);

			// Add text before rating
			for (const auto& ge : _ges)
			{
				ge->AddString(lpBeforeRating, false, m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint,
					cTemp, m_ssSettings.wcFontFace, m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f,
					m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f, cDropShadow);
			}

				// Add rating
				unsigned int lVolume = m_ITCom->GetVolumeLevel();

			RectF rfShape = RectF(0, 0, text_height*10.0f, text_height*1.5f);
			for (const auto& ge : _ges)
			{
				ge->AddShape(ShapeVolume, FillCustom, rfShape, false, cShapeOutline, 3.0, cShapeFill, lVolume);
				// Add text after rating
				ge->AddString(lpVolume + 8, false, m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint,
					cTemp, m_ssSettings.wcFontFace, m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f,
					m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f, cDropShadow);
			}

				delete[] lpBeforeRating;
		}
		else if (NULL != lpRating)
		{
			// Break up line
			lpBeforeRating = new wchar_t[lpRating - lpToken + 1];
			wcsncpy_s(lpBeforeRating, lpRating - lpToken + 1, lpToken, lpRating - lpToken);

			// Add text before rating
			for (const auto& ge : _ges)
			{
				ge->AddString(lpBeforeRating, false, m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint,
					cTemp, m_ssSettings.wcFontFace, m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f,
					m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f, cDropShadow);
			}

				// Add rating
				long lRating = m_ITCom->GetTrackRating();
			long lWhole = (lRating - (lRating % 20)) / 20;
			lRating = lRating - lWhole * 20;
			long lHalf = (lRating - (lRating % 10)) / 10;
			long lEmpty = 5 - lWhole - lHalf;

			RectF rfShape = RectF(0, 0, text_height, text_height);
			for (int i = 0; i < lWhole; i++)
			{
				for (const auto& ge : _ges)
				{
					if (m_ssSettings.nRatingType == 1)
						ge->AddImage(m_ssSettings.wcRatingFullPath);
					else
						ge->AddShape((ShapeType)m_ssSettings.nRatingShapeType, FillAll, rfShape, false, cShapeOutline, 3.0, cShapeFill);
				}
			}
			for (int i = 0; i < lHalf; i++)
			{
				for (const auto& ge : _ges)
				{
					if (m_ssSettings.nRatingType == 1)
						ge->AddImage(m_ssSettings.wcRatingHalfPath);
					else
						ge->AddShape((ShapeType)m_ssSettings.nRatingShapeType, (ShapeStyle)(m_ssSettings.nRatingShapeStyle + 2), rfShape, false, cShapeOutline, 3.0, cShapeFill);
				}
			}
			for (int i = 0; i < lEmpty; i++)
			{
				for (const auto& ge : _ges)
				{
					if (m_ssSettings.nRatingType == 1)
						ge->AddImage(m_ssSettings.wcRatingEmptyPath);
					else
						ge->AddShape((ShapeType)m_ssSettings.nRatingShapeType, FillNone, rfShape, false, cShapeOutline, 3.0, cShapeFill);
				}
			}

			// Add text after rating
			for (const auto& ge : _ges)
			{
				ge->AddString(lpRating + 8, false, m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint,
					cTemp, m_ssSettings.wcFontFace, m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f,
					m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f, cDropShadow);
			}

				delete[] lpBeforeRating;
		}
		else
		{
			for (const auto& ge : _ges)
			{
				ge->AddString(lpToken, false, m_ssSettings.nOSDFontStyle, (float)m_ssSettings.nOSDFontPoint,
					cTemp, m_ssSettings.wcFontFace, m_ssSettings.bOSDOutlineMode ? cOutline : cNull, 1.5f,
					m_ssSettings.bUseDropShadow ? m_ssSettings.fDropShadowOffset : 0.0f, cDropShadow);
			}
		}

		for (const auto& ge : _ges)
		{
			ge->EndLine();
		}

			lpToken = wcstok_s(NULL, L"\n", &lpContext);
	}

	// Position display
	if (!cm.GetMonitorRect(m_ssSettings.nOSDMonitor, rcScreen))
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

	pt.x = rcScreen.left + (LONG)ceilf((m_ssSettings.nOSDHoriz / 100.0f) * (rcScreen.right - rcScreen.left));
	pt.y = rcScreen.top + (LONG)ceilf((m_ssSettings.nOSDVert / 100.0f) * (rcScreen.bottom - rcScreen.top));

	// Show display, start timer
	if (m_ssSettings.bOSDAllMonitors)
	{
		int monitor = 1;

		for (const auto& ge : _ges)
		{
			// Position display
			cm.GetMonitorRect(monitor, rcScreen);

			pt.x = rcScreen.left + (LONG)ceilf((m_ssSettings.nOSDHoriz / 100.0f) * (rcScreen.right - rcScreen.left));
			pt.y = rcScreen.top + (LONG)ceilf((m_ssSettings.nOSDVert / 100.0f) * (rcScreen.bottom - rcScreen.top));

			ge->EndRender(pt, rcScreen);

			++monitor;
		}
	}
	else
	{
		for (const auto& ge : _ges)
		{
			ge->EndRender(pt, rcScreen);
		}
	}

	// Display should be shown for xx seconds, then hidden
	if (NULL != _hDisplayTimer)
		DeleteTimerQueueTimer(_hTimerQueue, _hDisplayTimer, INVALID_HANDLE_VALUE);
	_hDisplayTimer = NULL;
	CreateTimerQueueTimer(&_hDisplayTimer, _hTimerQueue, DisplayTimerCallback, (PVOID)this, display_time, 0, WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);

	DisplayShow();

	LeaveCriticalSection(&_csCreateDisplay);

	return;
}

VOID CALLBACK GraphicsInterface::DisplayTimerCallback(PVOID lpParameter, BOOLEAN)
{
	GraphicsInterface *gs = (GraphicsInterface*)lpParameter;

	gs->DisplayTimerCallback();

	return;
}

void GraphicsInterface::DisplayTimerCallback()
{
	RECT rcScreen;
	CMonitors cm;
	POINT pt = { 0, 0 };

	if (0 == TryEnterCriticalSection(&_csCreateDisplay))
		return;

	DeleteTimerQueueTimer(_hTimerQueue, _hDisplayTimer, NULL);
	_hDisplayTimer = NULL;

	if (m_ssSettings.bOSDAlwaysUp && _nDisplayType == 1 && !_override_always_up)
	{
		// Position display
		if (!cm.GetMonitorRect(m_ssSettings.nOSDMonitor, rcScreen))
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

		pt.x = rcScreen.left + (LONG)ceilf((m_ssSettings.nOSDHoriz / 100.0f) * (rcScreen.right - rcScreen.left));
		pt.y = rcScreen.top + (LONG)ceilf((m_ssSettings.nOSDVert / 100.0f) * (rcScreen.bottom - rcScreen.top));

		// Restore old info screen
		if (NULL == wcsstr(m_ssSettings.wcOSDFormat, L"%track_position%"))
		{
			for (const auto& ge : _ges)
			{
				ge->BeginRender();
			}

				if (m_ssSettings.bOSDAllMonitors)
				{
					int monitor = 1;

					for (const auto& ge : _ges)
					{
						// Position display
						cm.GetMonitorRect(monitor, rcScreen);

						pt.x = rcScreen.left + (LONG)ceilf((m_ssSettings.nOSDHoriz / 100.0f) * (rcScreen.right - rcScreen.left));
						pt.y = rcScreen.top + (LONG)ceilf((m_ssSettings.nOSDVert / 100.0f) * (rcScreen.bottom - rcScreen.top));

						ge->EndRender(pt, _bmInfo, rcScreen);

						++monitor;
					}
				}
				else
				{
					for (const auto& ge : _ges)
					{
						ge->EndRender(pt, _bmInfo, rcScreen);
					}
				}

			DisplayShow();
		}
		_nDisplayType = 0;
	}
	else
		DisplayHide();

	LeaveCriticalSection(&_csCreateDisplay);

	return;
}