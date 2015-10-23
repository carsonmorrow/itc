#include "InputEngine.h"
using namespace Gdiplus;
#include "memcheck.h"
#include <math.h>

ULONG_PTR gdiplusToken;
HANDLE hInputThread = NULL;

DWORD WINAPI InputThreadProc(LPVOID)
{
	WNDCLASSEX wc = { 0 };
	static TCHAR szAppName[] = _T("iTC_Input");

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = InputWinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szAppName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW, szAppName, _T("iTC_Input_Window"), WS_POPUP, 0,
		0, 0, 0, NULL, NULL, g_hInstance, NULL);

	inputWindow->SetHwnd(hWnd);
	m_hWndInput = hWnd;

	ShowWindow(m_hWndInput, SW_HIDE);
	SetWindowPos(m_hWndInput, HWND_TOPMOST, 0, 0, 300, 30, SWP_HIDEWINDOW);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK InputWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return inputWindow->WndProc(hwnd, message, wParam, lParam);
}

void InitInputEngine()
{
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	hInputThread = CreateThread(NULL, 0, InputThreadProc, NULL, 0, NULL);

	while (m_hWndInput == NULL)
	{
		Sleep(100);
	}

	return;
}

void ShutdownInputEngine()
{
	delete inputWindow;
	m_hWndInput = NULL;

	GdiplusShutdown(gdiplusToken);

	return;
}

InputWindow::InputWindow()
{
	_nAlpha = 0;
	_hwnd = 0;
}

int InputWindow::DisplayInput(inputView ivPage)
{
	_ivPage = ivPage;

	_hDone = CreateEvent(NULL, TRUE, FALSE, L"InputDoneEvent");

	DisplayInputInternal();

	WaitForSingleObject(_hDone, INFINITE);
	CloseHandle(_hDone);

	_ivPage.iiList = NULL;

	return _ivPage.nPos;
}
LRESULT InputWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (NULL == _hwnd) _hwnd = hwnd;

	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = NULL;

		hdc = BeginPaint(_hwnd, &ps);
		EndPaint(_hwnd, &ps);

		return true;
		break;
	}
	case WM_KEYDOWN:
	{
		// This should never happen, but crash report says it can
		if (_ivPage.nItems == 0)
			return true;

		if (VK_DOWN == wParam)
		{
			if (_ivPage.nPos == _ivPage.nItems - 1)
				_ivPage.nPos = _ivPage.nItems - 1;
			else
			{
				_ivPage.nPos++;
				this->DisplayInputInternal();
			}
		}
		else if (VK_UP == wParam)
		{
			if (_ivPage.nPos == 0)
				_ivPage.nPos = 0;
			else
			{
				_ivPage.nPos--;
				this->DisplayInputInternal();
			}
		}
		else if (VK_LEFT == wParam)
		{
			if (_ivPage.iiList[_ivPage.nPos].nType == 1 || _ivPage.iiList[_ivPage.nPos].nType == 2)
			{
				if (_ivPage.iiList[_ivPage.nPos].nValue > _ivPage.iiList[_ivPage.nPos].nMin)
				{
					_ivPage.iiList[_ivPage.nPos].nValue--;
					this->DisplayInputInternal();
				}
			}
		}
		else if (VK_RIGHT == wParam)
		{
			if (_ivPage.iiList[_ivPage.nPos].nType == 1 || _ivPage.iiList[_ivPage.nPos].nType == 2)
			{
				if (_ivPage.iiList[_ivPage.nPos].nValue < _ivPage.iiList[_ivPage.nPos].nMax)
				{
					_ivPage.iiList[_ivPage.nPos].nValue++;
					this->DisplayInputInternal();
				}
			}
		}
		else if (VK_BACK == wParam)
		{
			if (_ivPage.iiList[_ivPage.nPos].nType == 3)
			{
				_ivPage.iiList[_ivPage.nPos].wstrTitle = _ivPage.iiList[_ivPage.nPos].wstrTitle.substr(0, _ivPage.iiList[_ivPage.nPos].wstrTitle.length() - 1);
				this->DisplayInputInternal();
			}
		}
		else if (VK_SPACE == wParam)
		{
			if (_ivPage.iiList[_ivPage.nPos].nType == 3)
			{
				_ivPage.iiList[_ivPage.nPos].wstrTitle += ' ';
				this->DisplayInputInternal();
			}
		}
		else if (VK_ESCAPE == wParam)
		{
			_ivPage.nPos = -1;
			ShowWindow(_hwnd, SW_HIDE);
			SetEvent(_hDone);
		}
		else if (VK_RETURN == wParam)
		{
			if (_ivPage.iiList[_ivPage.nPos].nType != -1)
			{
				ShowWindow(_hwnd, SW_HIDE);
				SetEvent(_hDone);
			}
		}
		else
		{
			unsigned char ucKeyState[256];
			wchar_t wcBuffer[5] = { 0 };

			GetKeyboardState(ucKeyState);
			if (1 == ToUnicode(wParam, lParam, ucKeyState, wcBuffer, 5, 0))
			{
				if (_ivPage.iiList[_ivPage.nPos].nType == 3)
				{
					_ivPage.iiList[_ivPage.nPos].wstrTitle += wcBuffer;
					this->DisplayInputInternal();
				}
			}
		}

		return true;
		break;
	}
	default:
		return DefWindowProc(_hwnd, uMsg, wParam, lParam);
	}
}

void InputWindow::DisplayInputInternal()
{
	//Are we allowed to show anything?
	if (m_ssSettings.bVisualFeedback == false)
	{
		_ivPage.nPos = -1;
		SetEvent(_hDone);

		return;
	}

	HDC hdc = GetDC(_hwnd); //Grab a handle to the DC for our window
	Graphics graphics(hdc); //Create graphics object to measure string

	POINT point = { 0 };
	RECT rcScreen; //Size of screen
	RectF rfTitle; //Rect containing text
	RectF *rfItems = new RectF[_ivPage.nItems];
	SizeF szTitle; //Size of text
	SizeF *szItems = new SizeF[_ivPage.nItems];
	Size szOSD(0, 0); //Size of OSD
	PointF origin(0.0f, 0.0f);
	RectF rectTitle; //New bounding rect for text
	RectF *rectItems = new RectF[_ivPage.nItems];
	StringFormat stringFormat;
	Font *font = NULL;
	Font *fontBold = NULL;

	//Create font object
	font = new Font(m_ssSettings.wcFontFace, (float)m_ssSettings.nOSDFontPoint);
	fontBold = new Font(m_ssSettings.wcFontFace, (float)m_ssSettings.nOSDFontPoint, FontStyleBold);
	if (font->IsAvailable() == FALSE)
	{
		delete font;
		delete fontBold;
		font = new Font(FontFamily::GenericSansSerif(), (float)m_ssSettings.nOSDFontPoint);
		fontBold = new Font(FontFamily::GenericSansSerif(), (float)m_ssSettings.nOSDFontPoint, FontStyleBold);
		MessageBox(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), L"Windows had an error finding the font you selected for the OSD. iTunesControl will use a default font instead. Please select a different font immediately.", L"OSD Error", MB_OK | MB_ICONWARNING);
	}

	//Measure size of bounding rect for text
	graphics.MeasureString(_ivPage.wstrTitle.c_str(), -1, fontBold, origin, &rfTitle);
	rfTitle.GetSize(&szTitle);

	//this can also be used for pre-processing
	for (int i = 0; i < _ivPage.nItems; i++)
	{
		if (_ivPage.iiList[i].nType == 1)
		{
			wchar_t wcBuf[20] = { 0 };
			wsprintf(wcBuf, L"« %d »", _ivPage.iiList[i].nValue);
			_ivPage.iiList[i].wstrTitle = wcBuf;
		}
		else if (_ivPage.iiList[i].nType == 2)
		{
			_ivPage.iiList[i].wstrTitle = L"« ";
			_ivPage.iiList[i].wstrTitle += _ivPage.iiList[i].wstrSelect[_ivPage.iiList[i].nValue];
			_ivPage.iiList[i].wstrTitle += L" »";
		}
		graphics.MeasureString(_ivPage.iiList[i].wstrTitle.c_str(), -1, font, origin, &rfItems[i]);
		rfItems[i].GetSize(&szItems[i]);
	}

	//Set format flags, we use these in a couple places
	stringFormat.SetAlignment(StringAlignmentCenter);
	stringFormat.SetLineAlignment(StringAlignmentCenter);
	stringFormat.SetTrimming(StringTrimmingEllipsisCharacter);
	stringFormat.SetFormatFlags(StringFormatFlagsNoWrap);

	//Calculate rects for all objects
	int nPadding = 5;
	int nWidth = (int)ceilf(szTitle.Width);
	int nHeight = (int)ceilf(szTitle.Height);

	rectTitle = RectF(nPadding + 15.0f, nPadding + 15.0f, szTitle.Width, szTitle.Height);

	for (int i = 0; i < _ivPage.nItems; i++)
	{
		rectItems[i] = RectF(nPadding + 15.0f, nHeight + nPadding*(3 + 2 * i) + 15.0f, szItems[i].Width, szItems[i].Height);
		nHeight += (int)ceilf(szItems[i].Height);
		nWidth = max(nWidth, (int)ceilf(szItems[i].Width));
	}

	nWidth++; //Account for rounding errors
	rectTitle.Width = (float)nWidth;
	for (int i = 0; i < _ivPage.nItems; i++)
	{
		rectItems[i].Width = (float)nWidth;
	}

	szOSD.Width = nWidth + 2 * nPadding + 30; //6px padding on each side + 15px frame on each side
	szOSD.Height = nHeight + 2 * (_ivPage.nItems + 1)/*# of items*/*nPadding + 30; //6px padding on each side + 15px frame on each side

	//What's the RECT for our monitor?
	CMonitors cm;
	if (!cm.GetMonitorRect(m_ssSettings.nOSDMonitor, rcScreen)) //if this doesn't work, fall back on old method
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
	}

	point.x = (LONG)ceilf((rcScreen.right - rcScreen.left) * 0.5f - (szOSD.Width / 2.0f) + rcScreen.left);
	//point.x needs to be the px offset from left edge of monitor

	point.y = (LONG)ceilf((rcScreen.bottom - rcScreen.top) * 0.5f + rcScreen.top);
	//point.y needs to be the px offset from top edge of monitor
	//**************

	//Create buffer graphics object from bitmap
	Bitmap bmp(szOSD.Width, szOSD.Height, PixelFormat32bppPARGB);
	Graphics buffer(&bmp);
	buffer.SetPixelOffsetMode(PixelOffsetModeHalf);
	buffer.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	buffer.SetPageUnit(UnitPixel);
	buffer.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	//Create mask, background, and text brushes
	SolidBrush maskBrush(Color(0, GetRValue(m_ssSettings.lOSDBkColorRef), GetGValue(m_ssSettings.lOSDBkColorRef), GetBValue(m_ssSettings.lOSDBkColorRef)));
	SolidBrush backBrush(Color(GetRValue(m_ssSettings.lOSDBkColorRef), GetGValue(m_ssSettings.lOSDBkColorRef), GetBValue(m_ssSettings.lOSDBkColorRef)));
	SolidBrush solidBrush(Color(GetRValue(m_ssSettings.lOSDColorRef), GetGValue(m_ssSettings.lOSDColorRef), GetBValue(m_ssSettings.lOSDColorRef)));
	SolidBrush borderBrush(Color(GetRValue(m_ssSettings.lOSDBorderColor), GetGValue(m_ssSettings.lOSDBorderColor), GetBValue(m_ssSettings.lOSDBorderColor)));
	Pen borderPen(&borderBrush, /*m_ssSettings.nOSDBorderSize*/1);

	//Fill the masking rect
	buffer.FillRectangle(&maskBrush, 0, 0, szOSD.Width, szOSD.Height);
	buffer.FillRectangle(&backBrush, 15, 15, szOSD.Width - 30, szOSD.Height - 30);
	buffer.DrawRectangle(&borderPen, 15, 15, szOSD.Width - 30, szOSD.Height - 30);
	buffer.DrawRectangle(&borderPen, 15, 15, szOSD.Width - 30, (int)szTitle.Height + nPadding * 2 - 2);

	buffer.DrawString(_ivPage.wstrTitle.c_str(), -1, fontBold, rectTitle, &stringFormat, &solidBrush);

	for (int i = 0; i < _ivPage.nItems; i++)
	{
		if (_ivPage.nPos == i)
		{
			buffer.FillRectangle(&solidBrush, rectItems[i]);
			buffer.DrawString(_ivPage.iiList[i].wstrTitle.c_str(), -1, font, rectItems[i], &stringFormat, &backBrush);
		}
		else
			buffer.DrawString(_ivPage.iiList[i].wstrTitle.c_str(), -1, font, rectItems[i], &stringFormat, &solidBrush);
	}

	BitmapIntoLayeredWindow(_hwnd, bmp, point);

	delete[] rfItems;
	delete[] szItems;
	delete[] rectItems;
	delete font;
	delete fontBold;

	SetForegroundWindow(_hwnd);
	SetWindowPos(_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	SetForegroundWindow(_hwnd);

	return;
}

UINT InputWindow::BitmapIntoLayeredWindow(HWND hWnd, Bitmap &bmp, POINT &point)
{
	HDC screenDC(GetDC(0));
	POINT sourcePos = { 0 };
	HDC sourceDC(CreateCompatibleDC(screenDC));

	HBITMAP bufferBitmap = { 0 };
	bmp.GetHBITMAP(0, &bufferBitmap);

	HBITMAP oldBmpSelInDC = (HBITMAP)SelectObject(sourceDC, bufferBitmap);

	SIZE size = { static_cast<LONG>(bmp.GetWidth()),static_cast<LONG>(bmp.GetHeight()) };

	BLENDFUNCTION blendFunction = { 0 };
	blendFunction.BlendOp = AC_SRC_OVER;
	blendFunction.SourceConstantAlpha = (BYTE)((100.0f - m_ssSettings.nOSDAlpha) / 100.0f * 255.0f);
	blendFunction.AlphaFormat = AC_SRC_ALPHA;

	DWORD flags = ULW_ALPHA;

	UINT res = UpdateLayeredWindow(hWnd, NULL, &point, &size, sourceDC, &sourcePos, 0, &blendFunction, flags);

	SelectObject(sourceDC, oldBmpSelInDC);
	DeleteObject(bufferBitmap);
	DeleteDC(sourceDC);

	ReleaseDC(0, screenDC);

	return res;
}

void InputWindow::SetHwnd(HWND hwnd)
{
	_hwnd = hwnd;
	return;
}