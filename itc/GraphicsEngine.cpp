#include "GraphicsEngine.h"
#include "memcheck.h"
GraphicsEngine::GraphicsEngine(int id)
{
	// Create window
	swprintf_s(_wcDisplayWindowTitle, 32, L"iTC_GE_Wnd_%d", id);
	_hwndDisplayWindow = NULL;
	_hWindowThread = CreateThread(NULL, 0, WindowThreadProc, (LPVOID)this, 0, NULL);
	while (NULL == _hwndDisplayWindow) { Sleep(1); };

	// Init vars
	renderList = new RenderList();
	currentRLPosition = renderList->GetRoot();
	_bSiblingEntry = true;
	_bAutoEntrySet = false;
	_nAlpha = 0;
	_uiScheduleTimer = 0;
	_ndInfoBox = NULL;
	_ndArtBox = NULL;
}

GraphicsEngine::~GraphicsEngine()
{
	delete renderList;

	// Ask window to close
	PostMessage(_hwndDisplayWindow, WM_USER, 0, 0);
	// Wait for thread to exit
	if (WAIT_TIMEOUT == WaitForSingleObject(_hWindowThread, 1000))
		TerminateThread(_hWindowThread, (DWORD)-1);
}

void GraphicsEngine::BeginRender(int nDisplayLayout)
{
	// Set up internal structures for new renderlist
	renderList->ClearList();
	currentRLPosition = renderList->GetRoot();
	_ndArtBox = NULL;
	_ndInfoBox = NULL;

	_bAutoEntrySet = false;

	switch (nDisplayLayout)
	{
	case 0: // Art to left of text
		_ndArtBox = this->AddBox(false, false, 0.0f, 0.0f); // album art
		_ndInfoBox = this->AddBox(false, false, 8.0f, 8.0f); // info
		break;
	case 1: // Art above text
		_ndArtBox = this->AddBox(false, false, 0.0f, 0.0f); // album art
		_ndInfoBox = this->AddBox(true, false, 8.0f, 8.0f); // info
		break;
	case 2: // Art below text
		_ndInfoBox = this->AddBox(false, false, 8.0f, 8.0f); // info
		_ndArtBox = this->AddBox(true, false, 0.0f, 0.0f); // album art
		currentRLPosition = _ndInfoBox;
		break;
	case 3: // Art to right of text
		_ndInfoBox = this->AddBox(false, false, 8.0f, 8.0f); // info
		_ndArtBox = this->AddBox(false, false, 0.0f, 0.0f); // album art
		currentRLPosition = _ndInfoBox;
		break;
	case 4: // Art only
		_ndArtBox = this->AddBox(false, false, 0.0f, 0.0f); // album art
		break;
	default: // 5: Text only
		_ndInfoBox = this->AddBox(false, false, 8.0f, 8.0f); // info
		break;
	}

	_nDisplayLayout = nDisplayLayout;

	return;
}

void GraphicsEngine::EndRender(POINT point, Bitmap *bmp, RECT rcScreen)
{
	// Don't know why this would happen, but crash reports say it does
	if (!bmp)
		return;

	// Figure out where exactly we'll be putting the display
	point.x -= bmp->GetWidth() / 2;
	if (point.x + (int)bmp->GetWidth() > rcScreen.right)
		point.x = rcScreen.right - bmp->GetWidth();
	if (point.x < rcScreen.left)
		point.x = rcScreen.left;

	point.y -= bmp->GetHeight() / 2;
	if (point.y + (int)bmp->GetHeight() > rcScreen.bottom)
		point.y = rcScreen.bottom - bmp->GetHeight();
	if (point.y < rcScreen.top)
		point.y = rcScreen.top;

	// Restore saved display
	BitmapIntoLayeredWindow(_hwndDisplayWindow, *bmp, point);

	return;
}

double GraphicsEngine::GetLineHeights(void)
{
	double dLineHeight = 0.0f;
	double dChildHeight = 0.0f;
	Node *line = _ndInfoBox->child;
	Node *child = NULL;

	while (NULL != line)
	{
		child = line->child;
		dChildHeight = 0;
		while (NULL != child)
		{
			dChildHeight = (dChildHeight > child->rf->Height ? dChildHeight : child->rf->Height);
			child = child->next;
		}
		dLineHeight += dChildHeight;
		line = line->next;
	}

	return dLineHeight;
}

double GraphicsEngine::GetLineWidths(void)
{
	if (NULL == _ndInfoBox)
		return 0.0f;

	double dLineWidth = 0.0f;
	double dChildWidth = 0.0f;
	Node *line = _ndInfoBox->child;
	Node *child = NULL;

	// loop through all lines
	while (NULL != line)
	{
		child = line->child;
		dChildWidth = 0;
		// loop through all children of lines
		while (NULL != child)
		{
			dChildWidth += child->rf->Width;
			child = child->next;
		}
		dLineWidth = (dChildWidth > dLineWidth) ? dChildWidth : dLineWidth;
		line = line->next;
	}

	return dLineWidth;
}

void GraphicsEngine::AlignArt(void)
{
	if (!_ndArtBox || !_ndArtBox->rf || !_ndArtBox->rfBound)
		return;

	double dArtBox = _ndArtBox->rfBound->Width - _ndArtBox->rf->X;
	double dTextBox = _ndInfoBox->rfBound->Width - _ndInfoBox->rf->X;
	double dAdjustment = 0.0f;

	if (dArtBox < dTextBox)
	{
		if (_geoOptions.saAlign == StringAlignmentCenter)
			dAdjustment = floor((dTextBox - dArtBox) / 2.0);
		else
			dAdjustment = floor(dTextBox - dArtBox);

		_ndArtBox->rf->X += (float)dAdjustment;
		_ndArtBox->rfBound->Width += (float)dAdjustment;

		if (_ndArtBox->child && _ndArtBox->child->rf && _ndArtBox->child->rfBound)
		{
			_ndArtBox->child->rf->X += (float)dAdjustment;
			_ndArtBox->child->rfBound->Width += (float)dAdjustment;
		}
	}
}

void GraphicsEngine::AlignLines(void)
{
	if (NULL == _ndInfoBox)
		return;

	double dAdjustment = 0.0;
	double dBoxAdjust = 0.0;
	Node *line = _ndInfoBox->child;
	Node *child = NULL;

	// if art is wider than info
	if ((1 == _nDisplayLayout || 2 == _nDisplayLayout) && _ndArtBox->rfBound->Width > _ndInfoBox->rfBound->Width)
	{
		_ndInfoBox->rf->X = _ndArtBox->rf->X;
		_ndInfoBox->rfBound->Width = _ndArtBox->rfBound->Width;

	}

	// loop through all lines
	while (NULL != line)
	{
		if (_geoOptions.saAlign == StringAlignmentCenter)
			dAdjustment = (_ndInfoBox->rfBound->Width + _ndInfoBox->rf->X - line->rfBound->Width - line->rf->X) / 2.0 + dBoxAdjust;
		else
			dAdjustment = _ndInfoBox->rfBound->Width - line->rfBound->Width + dBoxAdjust;

		line->rf->X += (float)dAdjustment;
		line->rfBound->Width += (float)dAdjustment;

		child = line->child;

		// loop through all children of lines
		while (NULL != child)
		{
			child->rf->X += (float)dAdjustment;
			child->rfBound->Width += (float)dAdjustment;
			child = child->next;
		}

		line = line->next;
	}

	return;
}

void GraphicsEngine::AdjustLineWidths(double dMax)
{
	if (NULL == _ndInfoBox)
		return;

	double dAdjWidth = 0.0f;
	double dChildWidth = 0.0f;
	Node *line = _ndInfoBox->child;
	Node *child = NULL;
	Node *sibling = NULL;
	bool bDone = false;

	while (NULL != line)
	{
		child = line->child;
		dChildWidth = 0;
		while (NULL != child)
		{
			dChildWidth += child->rf->Width;
			child = child->next;
		}
		if (floor(dChildWidth) > ceil(dMax))
		{
			sibling = renderList->GetLastSibling(line->child);
			while (false == bDone && NULL != sibling)
			{
				if (sibling->rf->Width > (dChildWidth - dMax))
				{
					// Last sibling is solely responsible for length adjustment
					dAdjWidth = sibling->rf->Width - (dChildWidth - dMax);
					sibling->rf->Width = (float)dAdjWidth;
					sibling->rfBound->Width = (float)dAdjWidth;
					bDone = true;
				}
				else
				{
					// remove the sibling, and try again
					dChildWidth -= sibling->rf->Width;
					sibling->rf->Width = 0.0f;
					sibling->rfBound->Width = 0.0f;
					sibling = sibling->prev;
				}
			}
			bDone = false;
		}

		line = line->next;
	}

	return;
}

void GraphicsEngine::EndRender(POINT point, RECT rcScreen, bool bSave, Bitmap **bmpSave)
{
	HDC hdc = GetDC(_hwndDisplayWindow);
	Graphics *dcGraphics = new Graphics(hdc);
	GraphicsPath path;

	float padding = 8.0f;

	// We need a Graphics object to measure strings, so just use the one from our display DC
	renderList->MeasurePass(dcGraphics); // Measure out bounding rects

	// If album art is set to be bound to text (line) height, scale now
	if (_geoOptions.nArtScaleMode & ART_SCALEMODEHEIGHT && NULL != _ndArtBox && NULL != _ndArtBox->child && NULL != _ndInfoBox)
	{
		double dHeight = this->GetLineHeights();
		double dImageScale = 1.0f;

		ImageData *lpID = ((ImageData*)_ndArtBox->child->lpvData);

		delete lpID->rfSize;
		delete _ndArtBox->child->rf;
		delete _ndArtBox->child->rfBound;

		// Only scale if asked to, and don't divide by zero
		if (lpID->image->GetWidth() > 0 && lpID->image->GetHeight() > 0)
		{
			// Calculate scaling factor
			dImageScale = dHeight / lpID->image->GetHeight();
		}
		lpID->rfSize = new RectF(0, 0, floorf(lpID->image->GetWidth() * (float)dImageScale), floorf(lpID->image->GetHeight() * (float)dImageScale));
		_ndArtBox->child->rf = new RectF(0.0f, 0.0f, lpID->rfSize->Width, lpID->rfSize->Height);
		_ndArtBox->child->rfBound = new RectF(0.0f, 0.0f, lpID->rfSize->Width, lpID->rfSize->Height);
	}

	// If any lines are too long, crop now
	if (-1 != _geoOptions.nMaxWidth)
	{
		double dImgWidth = 0.0f;
		double dInfoWidth = 0.0f;

		if (NULL != _ndArtBox && NULL != _ndArtBox->child)
			dImgWidth = _ndArtBox->child->rf->Width;

		dInfoWidth = this->GetLineWidths();

		// First pass: text adjustment
		// Figure out width based on display type
		if ((1 == _nDisplayLayout || 2 == _nDisplayLayout || 5 == _nDisplayLayout) && (ceil(dInfoWidth) > _geoOptions.nMaxWidth))
		{
			// for all lines longer than max width, either remove elements or crop element rects
			this->AdjustLineWidths((double)_geoOptions.nMaxWidth);
		}
		// Figure out width based on display type
		else if ((0 == _nDisplayLayout || 3 == _nDisplayLayout) && ceil(dInfoWidth + dImgWidth + padding * 3) > _geoOptions.nMaxWidth)
		{
			// for all lines longer than max width, either remove elements or crop element rects
			this->AdjustLineWidths((double)_geoOptions.nMaxWidth - (dImgWidth + padding * 3));
		}

		// if the last pass didn't work, try it again with the art
		double dImageScale = 1.0f;

		if (1 == _nDisplayLayout || 2 == _nDisplayLayout || 4 == _nDisplayLayout)
			dInfoWidth = 0.0f;
		else
			dInfoWidth = this->GetLineWidths();

		if (ceil(dInfoWidth + dImgWidth + padding * 3) > _geoOptions.nMaxWidth && NULL != _ndArtBox && NULL != _ndArtBox->child)
		{
			ImageData *lpID = ((ImageData*)_ndArtBox->child->lpvData);

			if (lpID->image->GetWidth() > 0 || lpID->image->GetHeight() > 0)
			{
				// Calculate scaling factor
				dImageScale = (double)(_geoOptions.nMaxWidth - ceil(dInfoWidth + dImgWidth + padding * 3)) / (double)lpID->image->GetWidth();
			}

			delete lpID->rfSize;
			delete _ndArtBox->child->rf;
			delete _ndArtBox->child->rfBound;

			lpID->rfSize = new RectF(0, 0, lpID->image->GetWidth() * (float)dImageScale, lpID->image->GetHeight() * (float)dImageScale);
			_ndArtBox->child->rf = new RectF(0.0f, 0.0f, lpID->rfSize->Width, lpID->rfSize->Height);
			_ndArtBox->child->rfBound = new RectF(0.0f, 0.0f, lpID->rfSize->Width, lpID->rfSize->Height);
		}
	}

	renderList->PositionPass(dcGraphics); // Position bounding rects relative to renderlist

	// Fractional pixels cause problems, round up
	renderList->GetRoot()->rfBound->Width = ceilf(renderList->GetRoot()->rfBound->Width);
	renderList->GetRoot()->rfBound->Height = ceilf(renderList->GetRoot()->rfBound->Height);

	// Force to width if necessary
	if (_geoOptions.bForceToWidth && _geoOptions.nMaxWidth != -1)
	{
		// Art to left or right of text
		if (0 == _nDisplayLayout || 3 == _nDisplayLayout || 5 == _nDisplayLayout)
			_ndInfoBox->rfBound->Width += (_geoOptions.nMaxWidth - renderList->GetRoot()->rfBound->Width);
		else if (4 != _nDisplayLayout)
			_ndInfoBox->rfBound->Width = (float)_geoOptions.nMaxWidth;

		renderList->GetRoot()->rfBound->Width = (float)_geoOptions.nMaxWidth;
	}

	// Align "lines" if user desires
	if (_geoOptions.saAlign != StringAlignmentNear)
	{
		this->AlignLines();

		if (1 == _nDisplayLayout || 2 == _nDisplayLayout)
			this->AlignArt();
	}

	// We draw on this bitmap
	Bitmap bmp((int)renderList->GetRoot()->rfBound->Width, (int)renderList->GetRoot()->rfBound->Height, dcGraphics);
	Graphics buffer(&bmp);

	// Make the display more attractive
	buffer.SetCompositingQuality(CompositingQualityHighQuality);

	// Borders need defines called out
	RootData *lpRD = (RootData*)renderList->GetRoot()->lpvData;
	RectF rfBorder = *(renderList->GetRoot()->rfBound);

	// Draw background
	if (0 == _geoOptions.nBorderType)
	{
		// Rectangular border
		buffer.FillRectangle(lpRD->sbBkgnd, *(renderList->GetRoot()->rfBound));
		buffer.SetClip(*(renderList->GetRoot()->rfBound));
	}
	else {
		// Rounded rectangular border
		path.AddArc(rfBorder.X, rfBorder.Y, _geoOptions.fBorderRadius, _geoOptions.fBorderRadius, 180, 90);
		path.AddArc(rfBorder.GetRight() - _geoOptions.fBorderRadius, rfBorder.Y,
			_geoOptions.fBorderRadius, _geoOptions.fBorderRadius, 270, 90);
		path.AddArc(rfBorder.GetRight() - _geoOptions.fBorderRadius,
			rfBorder.GetBottom() - _geoOptions.fBorderRadius, _geoOptions.fBorderRadius, _geoOptions.fBorderRadius, 0, 90);
		path.AddArc(rfBorder.X, rfBorder.GetBottom() - _geoOptions.fBorderRadius,
			_geoOptions.fBorderRadius, _geoOptions.fBorderRadius, 90, 90);
		path.CloseFigure();
		buffer.FillPath(lpRD->sbBkgnd, &path);
		buffer.SetClip(&path);
	}

	// Make the display more attractive
	// Render types set smoothing modes as appropriate
	buffer.SetTextRenderingHint(TextRenderingHintSystemDefault);
	buffer.SetSmoothingMode(SmoothingModeNone);
	// Render layout tree
	renderList->RenderPass(&buffer);

	// Draw border
	if (0 == _geoOptions.nBorderType)
	{
		// Rectangular border
		buffer.SetSmoothingMode(SmoothingModeNone);
		lpRD->pnBorder->SetAlignment(PenAlignmentInset);
		buffer.DrawRectangle(lpRD->pnBorder, rfBorder);
	}
	else {
		// Rounded rectangular border
		buffer.SetSmoothingMode(SmoothingModeAntiAlias);
		buffer.ResetClip();
		buffer.DrawPath(lpRD->pnBorder, &path);
	}

	// Figure out where exactly we'll be putting the display
	point.x -= bmp.GetWidth() / 2;
	int nTemp = point.x + bmp.GetWidth();
	if (nTemp > rcScreen.right)
		point.x = rcScreen.right - bmp.GetWidth();
	if (point.x < rcScreen.left)
		point.x = rcScreen.left;

	point.y -= bmp.GetHeight() / 2;
	nTemp = point.y + bmp.GetHeight();
	if (nTemp > rcScreen.bottom)
		point.y = rcScreen.bottom - bmp.GetHeight();
	if (point.y < rcScreen.top)
		point.y = rcScreen.top;

	// Draw our display on the window
	BitmapIntoLayeredWindow(_hwndDisplayWindow, bmp, point);

	delete dcGraphics;
	dcGraphics = NULL;

	ReleaseDC(_hwndDisplayWindow, hdc);

	// Save display for the user if desired
	if (bSave && &bmpSave != NULL)
	{
		*bmpSave = bmp.Clone(0, 0, bmp.GetWidth(), bmp.GetHeight(), PixelFormatDontCare);
	}

	// Free memory used by render tree
	renderList->ClearList();
	currentRLPosition = renderList->GetRoot();

	return;
}

void GraphicsEngine::DisplayHide(bool bFade)
{
	if (bFade)
	{
		// Set timer for display fade out
		SetTimer(_hwndDisplayWindow, 2, 25, NULL);
	}
	else
	{
		// Set window display state
		ShowWindow(_hwndDisplayWindow, SW_HIDE);
		// Make sure we know our alpha value
		_nAlpha = 0;
		// Make sure we aren't doing any fade effects
		KillTimer(_hwndDisplayWindow, 2);
	}

	// Make sure we aren't also trying to fade in
	KillTimer(_hwndDisplayWindow, 3);

	return;
}

void GraphicsEngine::DisplayShow(bool bFade)
{
	if (bFade)
	{
		// Set the display to be at the same alpha we're at now to avoid flashing in
		BLENDFUNCTION blend;
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.AlphaFormat = AC_SRC_ALPHA;
		blend.SourceConstantAlpha = (BYTE)_nAlpha;
		UpdateLayeredWindow(_hwndDisplayWindow, NULL, NULL, NULL, NULL, NULL, NULL, &blend, ULW_ALPHA);

		// Make display visible for fade in
		if (_geoOptions.bDisplayTopMost)
			SetWindowPos(_hwndDisplayWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
		else
			SetWindowPos(_hwndDisplayWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
		// Set timer for display fade in
		SetTimer(_hwndDisplayWindow, 3, 25, NULL);
	}
	else
	{
		// Set window display state
		ShowWindow(_hwndDisplayWindow, SW_SHOWNA);
		// Make sure we know our alpha value
		_nAlpha = (int)((double)(100 - _geoOptions.nDisplayAlpha) / 100 * 255);
		// Make sure we aren't doing any fade effects
		KillTimer(_hwndDisplayWindow, 3);
	}

	// Make sure we aren't also trying to fade out
	KillTimer(_hwndDisplayWindow, 2);

	return;
}

// auto entry adds next item as child, then resets to sibling entry
void GraphicsEngine::SetEntryType(bool bSiblingEntry)
{
	_bSiblingEntry = bSiblingEntry;

	return;
}

void GraphicsEngine::AscendTree()
{
	// Move up a level in renderlist tree
	if (NULL != currentRLPosition->parent)
		currentRLPosition = currentRLPosition->parent;

	_bAutoEntrySet = false;

	return;
}

void GraphicsEngine::AddString(const wchar_t* tcString, bool bVertical, int nFontStyle, Gdiplus::REAL rSize, Color cColor, TCHAR *tcFont, Color cOutlineColor, REAL fOutlineWidth, float fDropShadowOffset, Color cDropShadow)
{
	Node *stringNode = NULL;

	// Add item as a child or sibling as appropriate
	if (renderList->GetRoot() == currentRLPosition || !_bSiblingEntry)
	{
		stringNode = renderList->AddChild(currentRLPosition);
		if (_bAutoEntrySet)
		{
			SetEntryType(true);
			_bAutoEntrySet = false;
		}
	}
	else
		stringNode = renderList->AddSibling(currentRLPosition);

	stringNode->nType = 1; // text
	stringNode->nFlags = (bVertical ? RL_LAYOUTVERT : 0);
	stringNode->pfPadding = new PointF(0.0f, 0.0f);
	TextData *td = new TextData;

	td->font = new Font(tcFont, rSize, nFontStyle);
	if (FALSE == td->font->IsAvailable())
	{
		delete td->font;
		td->font = new Font(FontFamily::GenericSansSerif(), rSize, nFontStyle);
	}

	td->fDropShadowOffset = fDropShadowOffset;
	td->drop = new SolidBrush(cDropShadow);
	td->saAlign = _geoOptions.saAlign;
	td->sb = new SolidBrush(cColor);
	td->sf = new StringFormat;
	td->pnOutline = new Pen(cOutlineColor, fOutlineWidth);
	td->tcText = (TCHAR *)calloc(_tcslen(tcString) + 1, sizeof(TCHAR));
	_tcscpy_s(td->tcText, _tcslen(tcString) + 1, tcString);
	stringNode->lpvData = td;

	currentRLPosition = stringNode;

	return;
}

REAL GraphicsEngine::GetTextHeight(TCHAR *name, REAL size, int style)
{
	Font *font = new Font(name, size, style);
	if (FALSE == font->IsAvailable())
	{
		delete font;
		font = new Font(FontFamily::GenericSansSerif(), size, style);
	}

	HDC hdc = GetDC(_hwndDisplayWindow);
	Graphics *graphics = new Graphics(hdc);
	REAL height = font->GetHeight(graphics);
	delete graphics;
	ReleaseDC(_hwndDisplayWindow, hdc);

	return height;
}

void GraphicsEngine::BeginLine(void)
{
	if (_ndInfoBox == currentRLPosition)
	{
		// Fake auto-entry for first line
		_bAutoEntrySet = true;
		this->SetEntryType(false);
	}

	this->AddBox(true);
}

void GraphicsEngine::EndLine(void)
{
	this->AscendTree();
}

Node *GraphicsEngine::AddBox(bool bVertical, bool bAutoEntryType, REAL rPaddingX, REAL rPaddingY)
{
	Node *boxNode = NULL;

	// Add item as a child or sibling as appropriate
	if (renderList->GetRoot() == currentRLPosition || !_bSiblingEntry)
	{
		boxNode = renderList->AddChild(currentRLPosition);
		if (_bAutoEntrySet)
		{
			SetEntryType(true);
			_bAutoEntrySet = false;
		}
	}
	else
		boxNode = renderList->AddSibling(currentRLPosition);

	boxNode->nType = 3;
	boxNode->nFlags = (bVertical ? RL_LAYOUTVERT : 0);
	boxNode->pfPadding = new PointF(rPaddingX, rPaddingY);

	currentRLPosition = boxNode;

	if (bAutoEntryType)
	{
		SetEntryType(false);
		_bAutoEntrySet = true;
	}

	return boxNode;
}

void GraphicsEngine::AddArt(TCHAR *tcPath, bool bVertical, Color cOutlineColor, REAL fOutlineWidth, REAL rMaxDimension, int nFlags)
{
	if (NULL == _ndArtBox)
		return;

	Node *ndTemp = currentRLPosition;

	currentRLPosition = _ndArtBox;

	this->SetEntryType(false); // Child
	this->AddImage(tcPath, bVertical, cOutlineColor, fOutlineWidth, rMaxDimension, nFlags);

	// Only add padding if art was added
	if (NULL != _ndArtBox->child)
	{
		_ndArtBox->pfPadding->X = 8.0f;
		_ndArtBox->pfPadding->Y = 8.0f;
	}

	this->SetEntryType(true); // Sibling

	currentRLPosition = ndTemp;

	return;
}

int GraphicsEngine::RoundUpPow2(int x)
{
	for (int y = 2; y < 8192; y *= 2)
	{
		if (x <= y)
			return y;
	}

	return 8192;
}

ULONG GraphicsEngine::CopyPNGToMemory(Bitmap* srcin, void* dest)
{
	IStream* inf = NULL;
	CreateStreamOnHGlobal(NULL, TRUE, &inf);

	int width = RoundUpPow2(srcin->GetWidth());
	int height = RoundUpPow2(srcin->GetHeight());

	Bitmap src(width, height, PixelFormat32bppARGB);

	if (srcin)
	{
		Graphics g(&src);
		g.SetCompositingMode(CompositingModeSourceCopy);
		g.DrawImage(srcin, 0, 0);
	}
	else
		return 0;

	if (inf)
	{
		CLSID encoder;
		GetEncoderClsid(L"image/png", &encoder);

		if (Ok == src.Save(inf, &encoder))
		{
			LARGE_INTEGER pos = { 0 };
			STATSTG stg = { 0 };
			HRESULT hr = inf->Stat(&stg, STATFLAG_NONAME);
			if (hr != S_OK)
			{
				inf->Release();
				return 0;
			}

			if (stg.cbSize.HighPart != 0)
			{
				inf->Release();
				return 0;
			}

			if (stg.cbSize.LowPart > 1048576)
			{
				inf->Release();
				return 0;
			}

			hr = inf->Seek(pos, STREAM_SEEK_SET, NULL);
			if (S_OK != hr)
			{
				inf->Release();
				return 0;
			}

			ULONG bytes_read;
			hr = inf->Read(dest, stg.cbSize.LowPart, &bytes_read);
			inf->Release();

			return bytes_read;
		}

		inf->Release();
	}

	return 0;
}

// http://cboard.cprogramming.com/windows-programming/110930-istream-release-access-violation-[cplusplus].html#post824400
Bitmap *GraphicsEngine::CreateGdiplusBitmapInStream(Bitmap* pbmSource)
{
	Bitmap *pbm = NULL;

	IStream *inf = NULL;
	CreateStreamOnHGlobal(NULL, TRUE, &inf);

	if (inf)
	{
		CLSID clsidEncoder;
		GetEncoderClsid(L"image/png", &clsidEncoder);

		if (Ok == pbmSource->Save(inf, &clsidEncoder))
			pbm = Bitmap::FromStream(inf);

		inf->Release();
	}

	if (pbm && pbm->GetLastStatus() != Ok)
	{
		delete pbm;
		pbm = NULL;
	}
	return pbm;
}

int GraphicsEngine::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void GraphicsEngine::AddImage(TCHAR *tcPath, bool bVertical, Color cOutlineColor, REAL fOutlineWidth, REAL rMaxDimension, int nFlags)
{
	if (NULL == tcPath || 0 == _tcscmp(tcPath, _T("")))
		return;

	Node *imageNode = NULL;
	Bitmap *bmTemp = NULL;
	double dImageScale = 1.0f;

	// Clone the bitmap into a memory stream so source file isn't locked
	bmTemp = new Bitmap(tcPath);

	if (Gdiplus::Ok != bmTemp->GetLastStatus())
		return;

	ImageData *id = new ImageData;
	id->image = CreateGdiplusBitmapInStream(bmTemp);
	delete bmTemp;
	bmTemp = NULL;
	if (NULL == id->image)
	{
		delete id;
		return;
	}

	// Add item as a child or sibling as appropriate
	if (renderList->GetRoot() == currentRLPosition || !_bSiblingEntry)
	{
		imageNode = renderList->AddChild(currentRLPosition);
		if (_bAutoEntrySet)
		{
			SetEntryType(true);
			_bAutoEntrySet = false;
		}
	}
	else
		imageNode = renderList->AddSibling(currentRLPosition);

	imageNode->nType = 2;
	imageNode->nFlags = (bVertical ? RL_LAYOUTVERT : 0);
	imageNode->pfPadding = new PointF(0.0f, 0.0f);

	id->pnOutline = new Pen(cOutlineColor, fOutlineWidth);

	// Only scale if asked to, and don't divide by zero
	if ((IMG_SCALEUP & nFlags || IMG_SCALEDOWN & nFlags) && (id->image->GetWidth() > 0 && id->image->GetHeight() > 0))
	{
		// Calculate scaling factor
		dImageScale = rMaxDimension / max(id->image->GetWidth(), id->image->GetHeight());
		// Check if we're allowed to scale in this direction
		if (dImageScale < 1.0f && !(nFlags & IMG_SCALEDOWN))
			dImageScale = 1.0f;
		if (dImageScale > 1.0f && !(nFlags & IMG_SCALEUP))
			dImageScale = 1.0f;
	}
	id->rfSize = new RectF(0, 0, floorf(id->image->GetWidth() * (float)dImageScale), floorf(id->image->GetHeight() * (float)dImageScale));
	imageNode->lpvData = id;

	currentRLPosition = imageNode;

	return;
}

void GraphicsEngine::AddShape(ShapeType type, ShapeStyle style, RectF &rfSize, bool bVertical, Color cOutlineColor, REAL fOutlineWidth, Color cColor, unsigned int uiCustomFill)
{
	Node *shapeNode = NULL;

	// Add item as a child or sibling as appropriate
	if (renderList->GetRoot() == currentRLPosition || !_bSiblingEntry)
	{
		shapeNode = renderList->AddChild(currentRLPosition);
		if (_bAutoEntrySet)
		{
			SetEntryType(true);
			_bAutoEntrySet = false;
		}
	}
	else
		shapeNode = renderList->AddSibling(currentRLPosition);

	shapeNode->nType = 4; // text
	shapeNode->nFlags = (bVertical ? RL_LAYOUTVERT : 0);
	shapeNode->pfPadding = new PointF(0.0f, 0.0f);
	ShapeData *sd = new ShapeData;

	sd->style = style;
	sd->type = type;
	sd->customFill = uiCustomFill;
	sd->sb = new SolidBrush(cColor);
	sd->outline = new Pen(cOutlineColor, fOutlineWidth);
	sd->size = new RectF(rfSize);
	shapeNode->lpvData = sd;
	currentRLPosition = shapeNode;

	return;
}

UINT GraphicsEngine::BitmapIntoLayeredWindow(HWND hWnd, Bitmap &bmp, POINT &point)
{
	HDC screenDC(GetDC(0));
	POINT sourcePos = { 0 };
	HDC sourceDC(CreateCompatibleDC(screenDC));

	HBITMAP bufferBitmap = { 0 };
	bmp.GetHBITMAP(0, &bufferBitmap);

	HBITMAP oldBmpSelInDC = (HBITMAP)SelectObject(sourceDC, bufferBitmap);

	SIZE size = { static_cast<LONG>(bmp.GetWidth()), static_cast<LONG>(bmp.GetHeight()) };

	BLENDFUNCTION blendFunction = { 0 };
	blendFunction.BlendOp = AC_SRC_OVER;
	blendFunction.SourceConstantAlpha = (BYTE)((double)(100 - _geoOptions.nDisplayAlpha) / 100 * 255);
	blendFunction.AlphaFormat = AC_SRC_ALPHA;

	DWORD flags = ULW_ALPHA;

	UINT res = UpdateLayeredWindow(hWnd, NULL, &point, &size, sourceDC, &sourcePos, 0, &blendFunction, flags);

	SelectObject(sourceDC, oldBmpSelInDC);
	DeleteObject(bufferBitmap);
	DeleteDC(sourceDC);

	ReleaseDC(0, screenDC);

	return res;
}

/*static*/ LRESULT GraphicsEngine::LightWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GraphicsEngine *pGE = (GraphicsEngine*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (NULL == pGE)
		return DefWindowProc(hwnd, message, wParam, lParam);
	else
		return pGE->GraphicsWindowProc(hwnd, message, wParam, lParam);
}

LRESULT GraphicsEngine::GraphicsWindowProc(HWND, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = NULL;

		hdc = BeginPaint(_hwndDisplayWindow, &ps);
		EndPaint(_hwndDisplayWindow, &ps);

		return true;
		break;
	}
	case WM_USER:
	{
		PostQuitMessage(0);
		return 0;
		break;
	}
	case WM_WINDOWPOSCHANGING:
	{
		LPWINDOWPOS lpwp = (LPWINDOWPOS)lParam;

		if (_geoOptions.bDisplayTopMost && (HWND_TOPMOST != lpwp->hwndInsertAfter || SWP_NOZORDER & lpwp->flags))
		{
			// we must be topmost
			lpwp->hwndInsertAfter = HWND_TOPMOST;
			// nozorder must not be set
			lpwp->flags &= (~SWP_NOZORDER);
		}
		else
		{
			// we must not be topmost
			lpwp->hwndInsertAfter = HWND_NOTOPMOST;
			// nozorder must not be set
			lpwp->flags &= (~SWP_NOZORDER);
		}

		return 0;
		break;
	}
	case WM_TIMER:
	{
		if (wParam == 3) //fade in
		{
			BLENDFUNCTION blend;
			blend.BlendOp = AC_SRC_OVER;
			blend.BlendFlags = 0;
			blend.AlphaFormat = AC_SRC_ALPHA;
			blend.SourceConstantAlpha = (BYTE)_nAlpha;

			if (_nAlpha < ((double)(100 - _geoOptions.nDisplayAlpha) / 100 * 255))
			{
				UpdateLayeredWindow(_hwndDisplayWindow, NULL, NULL, NULL, NULL, NULL, NULL, &blend, ULW_ALPHA);
				_nAlpha += 20; //this is for NEXT time
			}
			else if (_nAlpha != ((double)(100 - _geoOptions.nDisplayAlpha) / 100 * 255))
			{
				_nAlpha = blend.SourceConstantAlpha = (BYTE)((double)(100 - _geoOptions.nDisplayAlpha) / 100 * 255);
				UpdateLayeredWindow(_hwndDisplayWindow, NULL, NULL, NULL, NULL, NULL, NULL, &blend, ULW_ALPHA);
				KillTimer(_hwndDisplayWindow, 3);
			}
			else
			{
				KillTimer(_hwndDisplayWindow, 3);
			}

			return true;
		}

		if (wParam == 2) //fade out
		{
			BLENDFUNCTION blend;
			blend.BlendOp = AC_SRC_OVER;
			blend.BlendFlags = 0;
			blend.AlphaFormat = AC_SRC_ALPHA;
			blend.SourceConstantAlpha = (BYTE)_nAlpha;

			UpdateLayeredWindow(_hwndDisplayWindow, NULL, NULL, NULL, NULL, NULL, NULL, &blend, ULW_ALPHA);
			if (_nAlpha > 20) {
				_nAlpha -= 20;
			}
			else {
				_nAlpha = blend.SourceConstantAlpha = 0;
				UpdateLayeredWindow(_hwndDisplayWindow, NULL, NULL, NULL, NULL, NULL, NULL, &blend, ULW_ALPHA);
				ShowWindow(_hwndDisplayWindow, SW_HIDE);
				KillTimer(_hwndDisplayWindow, 2);
			}
			return true;
		}
	}
	default:
		return DefWindowProc(_hwndDisplayWindow, message, wParam, lParam);
	}
}

/*static*/ DWORD WINAPI GraphicsEngine::WindowThreadProc(LPVOID lpParam)
{
	return ((GraphicsEngine*)lpParam)->WindowThreadProc();
}

DWORD GraphicsEngine::WindowThreadProc()
{
	WNDCLASSEX wc = { 0 };
	static TCHAR szAppName[] = L"GraphicsEngine";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = LightWinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szAppName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT, szAppName, _wcDisplayWindowTitle, WS_POPUP, 0,
		0, 0, 0, NULL, NULL, (HINSTANCE)GetModuleHandle(NULL), NULL);

	_hwndDisplayWindow = hWnd;

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(_hwndDisplayWindow, SW_HIDE);
	if (_geoOptions.bDisplayTopMost)
		SetWindowPos(_hwndDisplayWindow, HWND_TOPMOST, 0, 0, 300, 30, SWP_HIDEWINDOW);
	else
		SetWindowPos(_hwndDisplayWindow, HWND_NOTOPMOST, 0, 0, 300, 30, SWP_HIDEWINDOW);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}