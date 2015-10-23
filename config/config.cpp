#include "config.h"
#include "memcheck.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	ENABLE_LEAK_CHECK

		HANDLE hMutex = NULL;

	//check for previous instance first
	hMutex = CreateMutex(NULL, TRUE, L"iTunesControlSettingsMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(hMutex);
		return 0;
	}

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;

	BOOL bICCE = InitCommonControlsEx(&icce);

	// Start up GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

	vKeys = new vector<HotKeyItem>;
	vOriginal = new vector<HotKeyItem>;

	LoadSettings(vOriginal, &_ssOriginal);
	LoadSettings(vKeys, &m_ssSettings);

	//tell itc to unreg hotkeys temporarily
	HWND hWndITCTemp = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");
	SendMessage(hWndITCTemp, WM_HOTKEYSACTION, 1, 0);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DLGCONFIGMAIN), NULL, MainDlgProc);

	// Shut down GDI+
	Gdiplus::GdiplusShutdown(g_gdiplusToken);

	delete vKeys;
	delete vOriginal;

	//tell itc to rereg hotkeys
	SendMessage(hWndITCTemp, WM_HOTKEYSACTION, 0, 0);

	CoUninitialize();

	ReleaseMutex(hMutex);

	return 0;
}

INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int retVal = FALSE;
	static HICON hIcon;
	static HICON hIconSm;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		TV_INSERTSTRUCT tvinsert;

		tvinsert.hInsertAfter = TVI_ROOT;
		tvinsert.hParent = NULL;
		tvinsert.itemex.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
		tvinsert.itemex.pszText = L"Display";
		tvinsert.itemex.cChildren = 1;
		tvinsert.itemex.state = TVIS_EXPANDED;
		tvinsert.itemex.stateMask = TVIS_EXPANDED;
		htiDisplay = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);

		tvinsert.itemex.pszText = L"Hotkeys";
		htiHotkeys = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.mask = TVIF_TEXT;
		tvinsert.itemex.pszText = L"Miscellaneous";
		htiMisc = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.pszText = L"Startup and Shutdown";
		htiStartShut = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.pszText = L"Advanced";
		htiAdvanced = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);

		tvinsert.itemex.pszText = L"Album Art";
		tvinsert.hInsertAfter = TVI_LAST;
		tvinsert.hParent = htiDisplay;
		htiAlbumArt = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.pszText = L"Colors and Fonts";
		htiColorsFonts = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.pszText = L"Formatting";
		htiLayout = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.pszText = L"Position and Size";
		htiPosition = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.pszText = L"Ratings";
		htiRatings = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);

		tvinsert.itemex.pszText = L"Features";
		tvinsert.hInsertAfter = TVI_LAST;
		tvinsert.hParent = htiHotkeys;
		htiFeatures = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
		tvinsert.itemex.pszText = L"Song Search";
		htiSongSearch = (HTREEITEM)SendDlgItemMessage(hwndDlg, IDC_TRELIST, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);

		_hwndDisplay = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPDISPLAY), hwndDlg, DisplaySheetDlgProc);
		_hwndLayout = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPLAYOUT), hwndDlg, LayoutSheetDlgProc);
		_hwndHotkeys = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPHOTKEYS), hwndDlg, HotkeysSheetDlgProc);
		_hwndMisc = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPMISC), hwndDlg, MiscSheetDlgProc);
		_hwndAlbumArt = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPART), hwndDlg, ArtSheetDlgProc);
		_hwndStartShut = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPSTARTSHUT), hwndDlg, StartShutSheetDlgProc);
		_hwndColorsFonts = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DLGCOLORSFONTS), hwndDlg, ColorsFontsSheetDlgProc);
		_hwndPosition = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DLGPOSITION), hwndDlg, PositionSheetDlgProc);
		_hwndFeatures = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DLGFEATURES), hwndDlg, FeaturesSheetDlgProc);
		_hwndAdvanced = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPADVANCED), hwndDlg, AdvancedSheetDlgProc);
		_hwndSongSearch = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPSONGSEARCH), hwndDlg, SongSearchSheetDlgProc);
		_hwndRatings = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPRATINGS), hwndDlg, RatingsSheetDlgProc);

		ShowWindow(_hwndDisplay, SW_NORMAL);
		_hwndCurrentPage = _hwndDisplay;

		hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 32, 32, 0);
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 16, 16, 0);
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BTNCLOSE:
		{
			SendMessage(_hwndDisplay, WM_APPLY, NULL, NULL);
			SendMessage(_hwndLayout, WM_APPLY, NULL, NULL);
			SendMessage(_hwndHotkeys, WM_APPLY, NULL, NULL);
			SendMessage(_hwndMisc, WM_APPLY, NULL, NULL);
			SendMessage(_hwndAlbumArt, WM_APPLY, NULL, NULL);
			SendMessage(_hwndStartShut, WM_APPLY, NULL, NULL);
			SendMessage(_hwndColorsFonts, WM_APPLY, NULL, NULL);
			SendMessage(_hwndPosition, WM_APPLY, NULL, NULL);
			SendMessage(_hwndFeatures, WM_APPLY, NULL, NULL);
			SendMessage(_hwndAdvanced, WM_APPLY, NULL, NULL);
			SendMessage(_hwndSongSearch, WM_APPLY, NULL, NULL);
			SendMessage(_hwndRatings, WM_APPLY, NULL, NULL);

			vector<HotKeyItem>::iterator p = vKeys->begin();
			vector<HotKeyItem>::iterator q = vOriginal->begin();

			bool bFail = false;
			while (p != vKeys->end())
			{
				if (0 != memcmp((void*)&(*q), (void*)&(*p), sizeof(HotKeyItem)))
					bFail = true;

				++p;
				++q;
			}

			if (memcmp(&_ssOriginal, &m_ssSettings, sizeof(SettingsStruct)) != 0 || bFail)
			{
				if (false == m_ssSettings.bShowCfgSaveWarning || MessageBox(hwndDlg, L"Save changes before exiting?", L"iTunesControl Configuration",
					MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_APPLMODAL) == IDYES)
				{
					SaveSettings(vKeys, &m_ssSettings);

					HWND hWndITCTemp = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");
					if (hWndITCTemp != NULL)
						PostMessage(hWndITCTemp, WM_RELOADSETTINGS, NULL, NULL);
				}
			}

			PostQuitMessage(0);
			break;
		}
		case IDC_BTNAPPLY:
		{
			// Collect settings
			SendMessage(_hwndDisplay, WM_APPLY, NULL, NULL);
			SendMessage(_hwndLayout, WM_APPLY, NULL, NULL);
			SendMessage(_hwndHotkeys, WM_APPLY, NULL, NULL);
			SendMessage(_hwndMisc, WM_APPLY, NULL, NULL);
			SendMessage(_hwndAlbumArt, WM_APPLY, NULL, NULL);
			SendMessage(_hwndStartShut, WM_APPLY, NULL, NULL);
			SendMessage(_hwndColorsFonts, WM_APPLY, NULL, NULL);
			SendMessage(_hwndPosition, WM_APPLY, NULL, NULL);
			SendMessage(_hwndFeatures, WM_APPLY, NULL, NULL);
			SendMessage(_hwndAdvanced, WM_APPLY, NULL, NULL);
			SendMessage(_hwndSongSearch, WM_APPLY, NULL, NULL);
			SendMessage(_hwndRatings, WM_APPLY, NULL, NULL);

			// Save settings
			SaveSettings(vKeys, &m_ssSettings);

			// Tell iTC to update
			HWND hWndITCTemp = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");
			if (hWndITCTemp != NULL)
				PostMessage(hWndITCTemp, WM_RELOADSETTINGS, NULL, NULL);

			// Reset what we consider "original" settings
			delete vKeys;
			delete vOriginal;

			vKeys = new vector<HotKeyItem>;
			vOriginal = new vector<HotKeyItem>;

			LoadSettings(vOriginal, &_ssOriginal);
			LoadSettings(vKeys, &m_ssSettings);

			break;
		}
		case IDC_BTNRESETALL:
		{
			if (MessageBox(hwndDlg, L"This will reset all settings! This cannot be undone! Continue with reset?", L"iTunesControl Configuration",
				MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_APPLMODAL) == IDYES)
			{
				// Close all pages
				DestroyWindow(_hwndDisplay);
				DestroyWindow(_hwndLayout);
				DestroyWindow(_hwndHotkeys);
				DestroyWindow(_hwndMisc);
				DestroyWindow(_hwndAlbumArt);
				DestroyWindow(_hwndStartShut);
				DestroyWindow(_hwndColorsFonts);
				DestroyWindow(_hwndPosition);
				DestroyWindow(_hwndFeatures);
				DestroyWindow(_hwndAdvanced);
				DestroyWindow(_hwndSongSearch);
				DestroyWindow(_hwndRatings);

				// Delete config file
				wchar_t szPath[MAX_PATH];
				if (!IsPortable())
				{
					if (SUCCEEDED(SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, L"iTunesControl", szPath)))
					{
						PathAppend(szPath, L"config.xml");
						_wremove(szPath);
					}
				}
				else
				{
					GetModuleFileName(NULL, szPath, MAX_PATH);
					PathRemoveFileSpec(szPath);
					PathAppend((LPWSTR)szPath, L"config.xml");
					_wremove(szPath);

					// Recreate so IsPortable() still works
					pugi::xml_document doc;
					doc.load(WCONFSTR);
					doc.save_file(szPath);
				}

				// Load config file
				delete vKeys;
				delete vOriginal;

				vKeys = new vector<HotKeyItem>;
				vOriginal = new vector<HotKeyItem>;

				LoadSettings(vOriginal, &_ssOriginal);
				LoadSettings(vKeys, &m_ssSettings);

				// Create pages
				_hwndDisplay = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPDISPLAY), hwndDlg, DisplaySheetDlgProc);
				_hwndLayout = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPLAYOUT), hwndDlg, LayoutSheetDlgProc);
				_hwndHotkeys = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPHOTKEYS), hwndDlg, HotkeysSheetDlgProc);
				_hwndMisc = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPMISC), hwndDlg, MiscSheetDlgProc);
				_hwndAlbumArt = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPART), hwndDlg, ArtSheetDlgProc);
				_hwndStartShut = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPSTARTSHUT), hwndDlg, StartShutSheetDlgProc);
				_hwndColorsFonts = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DLGCOLORSFONTS), hwndDlg, ColorsFontsSheetDlgProc);
				_hwndPosition = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DLGPOSITION), hwndDlg, PositionSheetDlgProc);
				_hwndFeatures = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DLGFEATURES), hwndDlg, FeaturesSheetDlgProc);
				_hwndAdvanced = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPADVANCED), hwndDlg, AdvancedSheetDlgProc);
				_hwndSongSearch = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPSONGSEARCH), hwndDlg, SongSearchSheetDlgProc);
				_hwndRatings = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_PROPRATINGS), hwndDlg, RatingsSheetDlgProc);

				ShowWindow(_hwndDisplay, SW_NORMAL);
				_hwndCurrentPage = _hwndDisplay;
			}
			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_DESTROY:
	{
		DestroyIcon(hIcon);
		DestroyIcon(hIconSm);
		PostQuitMessage(0);
		return FALSE;
	}
	case WM_NOTIFY:
	{
		switch (LOWORD(wParam))
		{
		case IDC_TRELIST:
		{
			if (((LPNMHDR)lParam)->code == TVN_SELCHANGED)
			{
				HTREEITEM htiSelection = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
				if (htiSelection == htiLayout)
					SwitchPages(_hwndLayout);
				else if (htiSelection == htiDisplay)
					SwitchPages(_hwndDisplay);
				else if (htiSelection == htiHotkeys)
					SwitchPages(_hwndHotkeys);
				else if (htiSelection == htiMisc)
					SwitchPages(_hwndMisc);
				else if (htiSelection == htiStartShut)
					SwitchPages(_hwndStartShut);
				else if (htiSelection == htiColorsFonts)
					SwitchPages(_hwndColorsFonts);
				else if (htiSelection == htiAlbumArt)
					SwitchPages(_hwndAlbumArt);
				else if (htiSelection == htiPosition)
					SwitchPages(_hwndPosition);
				else if (htiSelection == htiFeatures)
					SwitchPages(_hwndFeatures);
				else if (htiSelection == htiAdvanced)
					SwitchPages(_hwndAdvanced);
				else if (htiSelection == htiSongSearch)
					SwitchPages(_hwndSongSearch);
				else if (htiSelection == htiRatings)
					SwitchPages(_hwndRatings);
			}
			break;
		}
		}
		break;
	}
	case WM_CLOSE:
	{
		SendMessage(_hwndDisplay, WM_APPLY, NULL, NULL);
		SendMessage(_hwndLayout, WM_APPLY, NULL, NULL);
		SendMessage(_hwndHotkeys, WM_APPLY, NULL, NULL);
		SendMessage(_hwndMisc, WM_APPLY, NULL, NULL);
		SendMessage(_hwndAlbumArt, WM_APPLY, NULL, NULL);
		SendMessage(_hwndStartShut, WM_APPLY, NULL, NULL);
		SendMessage(_hwndColorsFonts, WM_APPLY, NULL, NULL);
		SendMessage(_hwndPosition, WM_APPLY, NULL, NULL);
		SendMessage(_hwndFeatures, WM_APPLY, NULL, NULL);
		SendMessage(_hwndAdvanced, WM_APPLY, NULL, NULL);
		SendMessage(_hwndSongSearch, WM_APPLY, NULL, NULL);
		SendMessage(_hwndRatings, WM_APPLY, NULL, NULL);

		vector<HotKeyItem>::iterator p = vKeys->begin();
		vector<HotKeyItem>::iterator q = vOriginal->begin();

		bool bFail = false;
		while (p != vKeys->end())
		{
			if (0 != memcmp((void*)&(*q), (void*)&(*p), sizeof(HotKeyItem)))
				bFail = true;

			++p;
			++q;
		}

		if (memcmp(&_ssOriginal, &m_ssSettings, sizeof(SettingsStruct)) != 0 || bFail)
		{
			if (false == m_ssSettings.bShowCfgSaveWarning || MessageBox(hwndDlg, L"Save changes before exiting?", L"iTunesControl Configuration",
				MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_APPLMODAL) == IDYES)
			{
				SaveSettings(vKeys, &m_ssSettings);

				HWND hWndITCTemp = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");
				if (hWndITCTemp != NULL)
					PostMessage(hWndITCTemp, WM_RELOADSETTINGS, NULL, NULL);
			}
		}

		PostQuitMessage(0);
		retVal = 0;
		break;
	}
	}

	return retVal;
}

LRESULT CALLBACK PicPositionSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
	static RECT rcHover = { 0 };
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		HBRUSH hbrRect = CreateSolidBrush(RGB(255, 0, 0));
		HBRUSH hbrHover = CreateSolidBrush(RGB(100, 100, 100));
		int nVert = m_ssSettings.nOSDVert / 10 * 10;
		int nHoriz = m_ssSettings.nOSDHoriz / 10 * 10;
		RECT rc = { nHoriz + 10, nVert + 1, nHoriz + 1, nVert + 10 };
		RECT rcRgn = { 110, 0, 0, 110 };
		POINT ptGrid[47] = { 0,0, 0,110, 10,110, 10,0, 20,0, 20,110, 30,110, 30,0, 40,0, 40,110, 50,110, 50,0, 60,0,
							60,110, 70,110, 70,0, 80,0, 80,110, 90,110, 90,0, 100,0, 100,110, 110,110, 110,0,
							0,0, 0,10, 110,10, 110,20, 0,20, 0,30, 110,30, 110,40, 0,40, 0,50, 110,50, 110,60,
							0,60, 0,70, 110,70, 110,80, 0,80, 0,90, 110,90, 110,100, 0,100, 0,110, 110,110 };

		hdc = BeginPaint(hWnd, &ps);

		FillRect(hdc, &rcRgn, GetSysColorBrush(CTLCOLOR_DLG));
		Polyline(hdc, ptGrid, 47);

		if (!(rcHover.left == 0 && rcHover.right == 0 && rcHover.top == 0 && rcHover.bottom == 0))
			FillRect(hdc, &rcHover, hbrHover);

		FillRect(hdc, &rc, hbrRect);

		EndPaint(hWnd, &ps);

		DeleteObject((HGDIOBJ)hbrRect);
		DeleteObject((HGDIOBJ)hbrHover);

		return TRUE;
		break;
	}
	case WM_MOUSELEAVE:
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_CANCEL;
		tme.hwndTrack = hWnd;
		TrackMouseEvent(&tme);

		rcHover.bottom = 0;
		rcHover.top = 0;
		rcHover.left = 0;
		rcHover.right = 0;

		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

		break;
	}
	case WM_MOUSEMOVE:
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hWnd;
		TrackMouseEvent(&tme);

		POINTS ptHit = MAKEPOINTS(lParam);

		if (ptHit.x < 110 && ptHit.y < 110)
		{
			RECT rc = { ptHit.x / 10 * 10 + 10, ptHit.y / 10 * 10 + 1, ptHit.x / 10 * 10 + 1, ptHit.y / 10 * 10 + 10 };
			if (memcmp(&rcHover, &rc, sizeof(RECT)) != 0)
			{
				RedrawWindow(hWnd, &rcHover, NULL, RDW_INVALIDATE);
				memcpy(&rcHover, &rc, sizeof(RECT));
				RedrawWindow(hWnd, &rcHover, NULL, RDW_INVALIDATE);
			}
		}

		break;
	}
	case WM_LBUTTONDOWN:
	{
		POINTS ptHit = MAKEPOINTS(lParam);

		if (ptHit.x < 110 && ptHit.y < 110)
		{
			m_ssSettings.nOSDHoriz = ptHit.x / 10 * 10;
			m_ssSettings.nOSDVert = ptHit.y / 10 * 10;

			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		}

		return TRUE;
		break;
	}
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK PositionSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetWindowSubclass(GetDlgItem(hwndDlg, IDC_PICPOSITION), PicPositionSubclassProc, 0, NULL);

		SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"None");
		SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Small");
		SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Medium");
		SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Large");

		SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"None");
		SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Small");
		SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Medium");
		SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Large");

		if (!m_ssSettings.bOSDUseBorder || m_ssSettings.nOSDBorderSize == 0)
			SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_SETCURSEL, 0, 0);
		else if (m_ssSettings.nOSDBorderSize <= 1)
			SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_SETCURSEL, 1, 0);
		else if (m_ssSettings.nOSDBorderSize <= 3)
			SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_SETCURSEL, 2, 0);
		else
			SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_SETCURSEL, 3, 0);

		if (m_ssSettings.nArtBorderSize == 0)
			SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_SETCURSEL, 0, 0);
		else if (m_ssSettings.nArtBorderSize == 1)
			SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_SETCURSEL, 1, 0);
		else if (m_ssSettings.nArtBorderSize == 2)
			SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_SETCURSEL, 2, 0);
		else
			SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_SETCURSEL, 3, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Small");
		SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Medium");
		SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Large");
		SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Extra Large");
		SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Unlimited");
		SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Custom");

		EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), FALSE);
		SetDlgItemInt(hwndDlg, IDC_EDTCUSTOMDISPSIZE, m_ssSettings.nOSDMaxWidth, TRUE);

		if (m_ssSettings.nOSDMaxWidth == 280)
			SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_SETCURSEL, 0, 0);
		else if (m_ssSettings.nOSDMaxWidth == 480)
			SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_SETCURSEL, 1, 0);
		else if (m_ssSettings.nOSDMaxWidth == 800)
			SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_SETCURSEL, 2, 0);
		else if (m_ssSettings.nOSDMaxWidth == 1024)
			SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_SETCURSEL, 3, 0);
		else if (m_ssSettings.nOSDMaxWidth == -1)
		{
			SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_SETCURSEL, 4, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKFORCEWIDTH), FALSE);
		}
		else
		{
			SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_SETCURSEL, 5, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), TRUE);
		}

		if (m_ssSettings.bOSDForceWidth)
			CheckDlgButton(hwndDlg, IDC_CHKFORCEWIDTH, BST_UNCHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKFORCEWIDTH, BST_CHECKED);

		if (m_ssSettings.bOSDAllMonitors)
		{
			CheckDlgButton(hwndDlg, IDC_RDOALLMONITORS, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSINGLEMONITOR, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBMONITOR), FALSE);
		}
		else
		{
			CheckDlgButton(hwndDlg, IDC_RDOALLMONITORS, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSINGLEMONITOR, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBMONITOR), TRUE);
		}

		CMonitors cm;
		for (int i = 1; i <= cm.GetCount(); i++)
		{
			wchar_t wcItem[4] = { 0 };
			wsprintf(wcItem, L"%d", i);
			SendDlgItemMessage(hwndDlg, IDC_CMBMONITOR, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)wcItem);
		}

		if (m_ssSettings.nOSDMonitor <= cm.GetCount())
			SendDlgItemMessage(hwndDlg, IDC_CMBMONITOR, CB_SETCURSEL, m_ssSettings.nOSDMonitor - 1, 0);
		else
			SendDlgItemMessage(hwndDlg, IDC_CMBMONITOR, CB_SETCURSEL, 0, 0);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_RDOALLMONITORS:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_RDOALLMONITORS) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMBMONITOR), FALSE);
				CheckDlgButton(hwndDlg, IDC_RDOSINGLEMONITOR, BST_UNCHECKED);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMBMONITOR), TRUE);
				CheckDlgButton(hwndDlg, IDC_RDOSINGLEMONITOR, BST_CHECKED);
			}
			break;
		}
		case IDC_RDOSINGLEMONITOR:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_RDOSINGLEMONITOR) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMBMONITOR), TRUE);
				CheckDlgButton(hwndDlg, IDC_RDOALLMONITORS, BST_UNCHECKED);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMBMONITOR), FALSE);
				CheckDlgButton(hwndDlg, IDC_RDOALLMONITORS, BST_CHECKED);
			}
			break;
		}
		case IDC_CMBDISPLAYSIZE:
		{
			int nTemp = 0;

			nTemp = SendDlgItemMessage(hwndDlg, IDC_CMBDISPLAYSIZE, CB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKFORCEWIDTH), TRUE);

			if (nTemp == 0)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), FALSE);
				SetDlgItemInt(hwndDlg, IDC_EDTCUSTOMDISPSIZE, 280, FALSE);
			}
			else if (nTemp == 1)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), FALSE);
				SetDlgItemInt(hwndDlg, IDC_EDTCUSTOMDISPSIZE, 480, FALSE);
			}
			else if (nTemp == 2)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), FALSE);
				SetDlgItemInt(hwndDlg, IDC_EDTCUSTOMDISPSIZE, 800, FALSE);
			}
			else if (nTemp == 3)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), FALSE);
				SetDlgItemInt(hwndDlg, IDC_EDTCUSTOMDISPSIZE, 1024, FALSE);
			}
			else if (nTemp == 4)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), FALSE);
				SetDlgItemInt(hwndDlg, IDC_EDTCUSTOMDISPSIZE, -1, TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKFORCEWIDTH), FALSE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTCUSTOMDISPSIZE), TRUE);
			}

			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		int nTemp = SendDlgItemMessage(hwndDlg, IDC_CMBBORDERWIDTH, CB_GETCURSEL, 0, 0);

		if (nTemp == 0)
		{
			m_ssSettings.bOSDUseBorder = false;
			m_ssSettings.nOSDBorderSize = 0;
		}
		else if (nTemp == 1)
		{
			m_ssSettings.bOSDUseBorder = true;
			m_ssSettings.nOSDBorderSize = 2;
		}
		else if (nTemp == 2)
		{
			m_ssSettings.bOSDUseBorder = true;
			m_ssSettings.nOSDBorderSize = 3;
		}
		else
		{
			m_ssSettings.bOSDUseBorder = true;
			m_ssSettings.nOSDBorderSize = 5;
		}

		nTemp = SendDlgItemMessage(hwndDlg, IDC_CMBMONITOR, CB_GETCURSEL, 0, 0);
		m_ssSettings.nOSDMonitor = nTemp + 1;

		if (IsDlgButtonChecked(hwndDlg, IDC_RDOALLMONITORS) == BST_CHECKED)
			m_ssSettings.bOSDAllMonitors = true;
		else
			m_ssSettings.bOSDAllMonitors = false;

		nTemp = SendDlgItemMessage(hwndDlg, IDC_CMBARTBORDERWIDTH, CB_GETCURSEL, 0, 0);
		m_ssSettings.nArtBorderSize = nTemp;

		m_ssSettings.nOSDMaxWidth = GetDlgItemInt(hwndDlg, IDC_EDTCUSTOMDISPSIZE, NULL, TRUE);
		if (!(m_ssSettings.nOSDMaxWidth == -1 || m_ssSettings.nOSDMaxWidth > 0))
			m_ssSettings.nOSDMaxWidth = 800;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKFORCEWIDTH) == BST_CHECKED)
			m_ssSettings.bOSDForceWidth = false;
		else
			m_ssSettings.bOSDForceWidth = true;

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		RemoveWindowSubclass(GetDlgItem(hwndDlg, IDC_PICPOSITION), PicPositionSubclassProc, 0);
		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK ColorsFontsSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		g_hbrBkgnd = CreateSolidBrush(m_ssSettings.lOSDBkColorRef);
		g_hbrFont = CreateSolidBrush(m_ssSettings.lOSDColorRef);
		g_hbrBorder = CreateSolidBrush(m_ssSettings.lOSDBorderColor);
		g_hbrArtBorder = CreateSolidBrush(m_ssSettings.lArtBorderColor);
		g_hbrDropShadow = CreateSolidBrush(m_ssSettings.lDropShadowColor);
		SetDlgItemText(hwndDlg, IDC_COLOR_BK, L"");
		SetDlgItemText(hwndDlg, IDC_COLOR_FONT, L"");
		SetDlgItemText(hwndDlg, IDC_COLOR_BORDER, L"");
		SetDlgItemText(hwndDlg, IDC_COLOR_ART, L"");
		SetDlgItemText(hwndDlg, IDC_COLOR_DROP, L"");

		if (m_ssSettings.bOSDOutlineMode)
			CheckDlgButton(hwndDlg, IDC_CHKOUTLINEMODE, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKOUTLINEMODE, BST_UNCHECKED);

		if (m_ssSettings.bUseDropShadow)
			CheckDlgButton(hwndDlg, IDC_CHKUSEDROPSHADOW, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKUSEDROPSHADOW, BST_UNCHECKED);

		// FONT STYLE CONTROL
		//
		LOGFONT lf;
		HDC hDesktop = CreateDC(L"DISPLAY", NULL, NULL, NULL);

		memset(&lf, 0, sizeof(LOGFONT));
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		wcscpy_s(lf.lfFaceName, 32, m_ssSettings.wcFontFace);
		lf.lfHeight = -MulDiv(m_ssSettings.nOSDFontPoint, GetDeviceCaps(hDesktop, LOGPIXELSY), 72);
		lf.lfWidth = 0;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
		lf.lfWeight = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleBold) ? FW_BOLD : FW_DONTCARE;
		lf.lfItalic = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleItalic) ? TRUE : FALSE;
		lf.lfUnderline = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleUnderline) ? TRUE : FALSE;
		lf.lfStrikeOut = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleStrikeout) ? TRUE : FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

		SetWindowText(GetDlgItem(hwndDlg, IDC_FONT_STYLE), lf.lfFaceName);
		g_hFont = CreateFontIndirect(&lf);
		SendMessage(GetDlgItem(hwndDlg, IDC_FONT_STYLE), WM_SETFONT, (WPARAM)g_hFont, MAKELPARAM(TRUE, 0));

		DeleteDC(hDesktop);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_FONT_STYLE:
		{
			CHOOSEFONT cf;
			LOGFONT lf;
			HDC hDesktop = CreateDC(L"DISPLAY", NULL, NULL, NULL);

			ZeroMemory(&cf, sizeof(cf));
			cf.lStructSize = sizeof(cf);
			cf.hwndOwner = hwndDlg;
			cf.lpLogFont = &lf;
			cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

			lf.lfEscapement = 0;
			lf.lfOrientation = 0;
			wcscpy_s(lf.lfFaceName, 32, m_ssSettings.wcFontFace);
			lf.lfHeight = -MulDiv(m_ssSettings.nOSDFontPoint, GetDeviceCaps(hDesktop, LOGPIXELSY), 72);
			lf.lfWidth = 0;
			lf.lfQuality = DEFAULT_QUALITY;
			lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lf.lfOutPrecision = OUT_OUTLINE_PRECIS;
			lf.lfWeight = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleBold) ? FW_BOLD : FW_DONTCARE;
			lf.lfItalic = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleItalic) ? TRUE : FALSE;
			lf.lfUnderline = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleUnderline) ? TRUE : FALSE;
			lf.lfStrikeOut = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleStrikeout) ? TRUE : FALSE;
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

			if (ChooseFont(&cf) == TRUE)
			{
				wcscpy_s(m_ssSettings.wcFontFace, 32, lf.lfFaceName);
				m_ssSettings.nOSDFontPoint = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDesktop, LOGPIXELSY));
				m_ssSettings.nOSDFontStyle = 0;
				if (lf.lfWeight == FW_BOLD)
					m_ssSettings.nOSDFontStyle |= Gdiplus::FontStyleBold;
				if (lf.lfItalic != FALSE)
					m_ssSettings.nOSDFontStyle |= Gdiplus::FontStyleItalic;
				if (lf.lfUnderline != FALSE)
					m_ssSettings.nOSDFontStyle |= Gdiplus::FontStyleUnderline;
				if (lf.lfStrikeOut != FALSE)
					m_ssSettings.nOSDFontStyle |= Gdiplus::FontStyleStrikeout;

				SetWindowText(GetDlgItem(hwndDlg, IDC_FONT_STYLE), lf.lfFaceName);
				DeleteObject(g_hFont);
				lf.lfHeight = -MulDiv(m_ssSettings.nOSDFontPoint, GetDeviceCaps(hDesktop, LOGPIXELSY), 72);
				lf.lfWeight = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleBold) ? FW_BOLD : FW_DONTCARE;
				lf.lfItalic = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleItalic) ? TRUE : FALSE;
				lf.lfUnderline = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleUnderline) ? TRUE : FALSE;
				lf.lfStrikeOut = (m_ssSettings.nOSDFontStyle & Gdiplus::FontStyleStrikeout) ? TRUE : FALSE;
				g_hFont = CreateFontIndirect(&lf);
				SendMessage(GetDlgItem(hwndDlg, IDC_FONT_STYLE), WM_SETFONT, (WPARAM)g_hFont, MAKELPARAM(TRUE, 0));
			}

			DeleteDC(hDesktop);

			break;
		}
		case IDC_COLOR_FONT:
		{
			CHOOSECOLOR cc;
			COLORREF * CustColors = new COLORREF[16];

			CustColors[0] = m_ssSettings.lOSDColorRef;

			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.rgbResult = m_ssSettings.lOSDColorRef;
			cc.hwndOwner = hwndDlg;
			cc.lpCustColors = CustColors;
			cc.Flags = CC_ANYCOLOR | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				m_ssSettings.lOSDColorRef = cc.rgbResult;
				DeleteObject((HGDIOBJ)g_hbrFont);
				g_hbrFont = CreateSolidBrush(m_ssSettings.lOSDColorRef);
				RedrawWindow(GetDlgItem(hwndDlg, IDC_COLOR_FONT), NULL, NULL, RDW_INVALIDATE);
			}

			delete[] CustColors;

			break;
		}
		case IDC_COLOR_BK:
		{
			CHOOSECOLOR cc;
			COLORREF * CustColors = new COLORREF[16];

			CustColors[0] = m_ssSettings.lOSDBkColorRef;

			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.rgbResult = m_ssSettings.lOSDBkColorRef;
			cc.hwndOwner = hwndDlg;
			cc.lpCustColors = CustColors;
			cc.Flags = CC_ANYCOLOR | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				m_ssSettings.lOSDBkColorRef = cc.rgbResult;
				DeleteObject((HGDIOBJ)g_hbrBkgnd);
				g_hbrBkgnd = CreateSolidBrush(m_ssSettings.lOSDBkColorRef);
				RedrawWindow(GetDlgItem(hwndDlg, IDC_COLOR_BK), NULL, NULL, RDW_INVALIDATE);
			}

			delete[] CustColors;

			break;
		}
		case IDC_COLOR_BORDER:
		{
			CHOOSECOLOR cc;
			COLORREF * CustColors = new COLORREF[16];

			CustColors[0] = m_ssSettings.lOSDBorderColor;

			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.rgbResult = m_ssSettings.lOSDBorderColor;
			cc.hwndOwner = hwndDlg;
			cc.lpCustColors = CustColors;
			cc.Flags = CC_ANYCOLOR | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				m_ssSettings.lOSDBorderColor = cc.rgbResult;
				DeleteObject((HGDIOBJ)g_hbrBorder);
				g_hbrBorder = CreateSolidBrush(m_ssSettings.lOSDBorderColor);
				RedrawWindow(GetDlgItem(hwndDlg, IDC_COLOR_BORDER), NULL, NULL, RDW_INVALIDATE);
			}

			delete[] CustColors;

			break;
		}
		case IDC_COLOR_ART:
		{
			CHOOSECOLOR cc;
			COLORREF * CustColors = new COLORREF[16];

			CustColors[0] = m_ssSettings.lArtBorderColor;

			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.rgbResult = m_ssSettings.lArtBorderColor;
			cc.hwndOwner = hwndDlg;
			cc.lpCustColors = CustColors;
			cc.Flags = CC_ANYCOLOR | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				m_ssSettings.lArtBorderColor = cc.rgbResult;
				DeleteObject((HGDIOBJ)g_hbrArtBorder);
				g_hbrArtBorder = CreateSolidBrush(m_ssSettings.lArtBorderColor);
				RedrawWindow(GetDlgItem(hwndDlg, IDC_COLOR_ART), NULL, NULL, RDW_INVALIDATE);
			}

			delete[] CustColors;

			break;
		}
		case IDC_COLOR_DROP:
		{
			CHOOSECOLOR cc;
			COLORREF * CustColors = new COLORREF[16];

			CustColors[0] = m_ssSettings.lDropShadowColor;

			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.rgbResult = m_ssSettings.lDropShadowColor;
			cc.hwndOwner = hwndDlg;
			cc.lpCustColors = CustColors;
			cc.Flags = CC_ANYCOLOR | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				m_ssSettings.lDropShadowColor = cc.rgbResult;
				DeleteObject((HGDIOBJ)g_hbrDropShadow);
				g_hbrDropShadow = CreateSolidBrush(m_ssSettings.lDropShadowColor);
				RedrawWindow(GetDlgItem(hwndDlg, IDC_COLOR_DROP), NULL, NULL, RDW_INVALIDATE);
			}

			delete[] CustColors;

			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_CTLCOLORSTATIC:
	{
		int nCntrlID = GetDlgCtrlID((HWND)lParam);

		switch (nCntrlID)
		{
		case IDC_COLOR_BK:
			return (LRESULT)g_hbrBkgnd;
		case IDC_COLOR_FONT:
			return (LRESULT)g_hbrFont;
		case IDC_COLOR_BORDER:
			return (LRESULT)g_hbrBorder;
		case IDC_COLOR_ART:
			return (LRESULT)g_hbrArtBorder;
		case IDC_COLOR_DROP:
			return (LRESULT)g_hbrDropShadow;
		}

		return NULL;
	}
	case WM_APPLY:
	{
		if (IsDlgButtonChecked(hwndDlg, IDC_CHKOUTLINEMODE) == BST_CHECKED)
			m_ssSettings.bOSDOutlineMode = true;
		else
			m_ssSettings.bOSDOutlineMode = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKUSEDROPSHADOW) == BST_CHECKED)
			m_ssSettings.bUseDropShadow = true;
		else
			m_ssSettings.bUseDropShadow = false;

		return TRUE;
		break;
	}
	case WM_CLOSE:
	{
		DeleteObject((HGDIOBJ)g_hbrBkgnd);
		DeleteObject((HGDIOBJ)g_hbrFont);
		DeleteObject((HGDIOBJ)g_hbrBorder);
		DeleteObject((HGDIOBJ)g_hbrArtBorder);
		DeleteObject((HGDIOBJ)g_hbrDropShadow);
		DeleteObject((HGDIOBJ)g_hFont);

		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK DisplaySheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SendMessage(GetDlgItem(hwndDlg, IDC_SLDTRANSPARENCY), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
		SendMessage(GetDlgItem(hwndDlg, IDC_SLDTRANSPARENCY), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)m_ssSettings.nOSDAlpha);

		SetDlgItemInt(hwndDlg, IDC_EDTINFOTIME, m_ssSettings.nPopupShowTime / 1000, FALSE);
		SetDlgItemInt(hwndDlg, IDC_EDTINFOTIMEEND, m_ssSettings.nDisplayEndTime / 1000, FALSE);
		SetDlgItemInt(hwndDlg, IDC_EDTINFOTIMESTART, m_ssSettings.nDisplayStartTime / 1000, FALSE);
		SetDlgItemInt(hwndDlg, IDC_EDTOFFSETBEGIN, m_ssSettings.nShowBeginOffset / 1000, FALSE);
		SetDlgItemInt(hwndDlg, IDC_EDTOFFSETEND, m_ssSettings.nShowEndOffset / 1000, FALSE);
		SetDlgItemInt(hwndDlg, IDC_EDTDISABLESHORT, m_ssSettings.nShortTrackLength, FALSE);

		if (m_ssSettings.bOSDTopMost)
			CheckDlgButton(hwndDlg, IDC_CHKTOPMOST, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKTOPMOST, BST_UNCHECKED);

		if (m_ssSettings.bNoDisplayShortTracks)
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTDISABLESHORT), TRUE);
			CheckDlgButton(hwndDlg, IDC_CHKDISABLESHORTTRACKS, BST_CHECKED);
		}
		else
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTDISABLESHORT), FALSE);
			CheckDlgButton(hwndDlg, IDC_CHKDISABLESHORTTRACKS, BST_UNCHECKED);
		}

		if (m_ssSettings.bOSDHideWhenPaused)
			CheckDlgButton(hwndDlg, IDC_CHKEXCEPTWHENPAUSED, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKEXCEPTWHENPAUSED, BST_UNCHECKED);

		if (m_ssSettings.bOSDAlwaysUp)
		{
			CheckDlgButton(hwndDlg, IDC_RDOALWAYSSHOW, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHOWFORX, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKEXCEPTWHENPAUSED), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATEND), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATSTART), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), FALSE);
		}
		else
		{
			CheckDlgButton(hwndDlg, IDC_RDOALWAYSSHOW, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHOWFORX, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKEXCEPTWHENPAUSED), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIME), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATEND), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATSTART), TRUE);
			if (m_ssSettings.bShowDisplayAtEnd)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), FALSE);
			}
			if (m_ssSettings.bShowDisplayAtStart)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), FALSE);
			}
		}

		if (m_ssSettings.bFadeEffects)
			CheckDlgButton(hwndDlg, IDC_CHKUSEFADE, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKUSEFADE, BST_UNCHECKED);

		if (m_ssSettings.bVisualFeedback)
			CheckDlgButton(hwndDlg, IDC_CHKUSEVISUALS, BST_UNCHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKUSEVISUALS, BST_CHECKED);

		if (m_ssSettings.bNoDisplayWhenActive)
			CheckDlgButton(hwndDlg, IDC_CHKHIDEDISPLAYITUNES, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKHIDEDISPLAYITUNES, BST_UNCHECKED);

		if (m_ssSettings.bShowDisplayAtEnd)
			CheckDlgButton(hwndDlg, IDC_CHKSHOWATEND, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKSHOWATEND, BST_UNCHECKED);

		if (m_ssSettings.bShowDisplayAtStart)
			CheckDlgButton(hwndDlg, IDC_CHKSHOWATSTART, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKSHOWATSTART, BST_UNCHECKED);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_RDOALWAYSSHOW:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_RDOALWAYSSHOW) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKEXCEPTWHENPAUSED), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIME), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATSTART), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), FALSE);
				CheckDlgButton(hwndDlg, IDC_RDOSHOWFORX, BST_UNCHECKED);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKEXCEPTWHENPAUSED), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIME), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATEND), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATSTART), TRUE);
				if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATEND) == BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), TRUE);
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATSTART) == BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), TRUE);
				}
				CheckDlgButton(hwndDlg, IDC_RDOSHOWFORX, BST_CHECKED);
			}
			break;
		}
		case IDC_RDOSHOWFORX:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_RDOSHOWFORX) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIME), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATEND), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATSTART), TRUE);
				if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATEND) == BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), TRUE);
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATSTART) == BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), TRUE);
				}
				CheckDlgButton(hwndDlg, IDC_RDOALWAYSSHOW, BST_UNCHECKED);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIME), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHOWATSTART), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), FALSE);
				CheckDlgButton(hwndDlg, IDC_RDOALWAYSSHOW, BST_CHECKED);
			}
			break;
		}
		case IDC_CHKDISABLESHORTTRACKS:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_CHKDISABLESHORTTRACKS) == BST_CHECKED)
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTDISABLESHORT), TRUE);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTDISABLESHORT), FALSE);
			break;
		}
		case IDC_CHKSHOWATEND:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATEND) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETEND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMEEND), FALSE);
			}
			break;
		}
		case IDC_CHKSHOWATSTART:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATSTART) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTOFFSETBEGIN), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTINFOTIMESTART), FALSE);
			}
			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		int nTemp = 0;

		nTemp = SendMessage(GetDlgItem(hwndDlg, IDC_SLDTRANSPARENCY), TBM_GETPOS, 0, 0);
		if (nTemp < 0)
			nTemp = 0;
		else if (nTemp > 100)
			nTemp = 100;
		m_ssSettings.nOSDAlpha = nTemp;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTINFOTIME, FALSE, FALSE);
		if (nTemp < 1)
			nTemp = 1;
		else if (nTemp > 3600)
			nTemp = 3600;
		m_ssSettings.nPopupShowTime = nTemp * 1000;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTINFOTIMEEND, FALSE, FALSE);
		if (nTemp < 1)
			nTemp = 1;
		else if (nTemp > 3600)
			nTemp = 3600;
		m_ssSettings.nDisplayEndTime = nTemp * 1000;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTINFOTIMESTART, FALSE, FALSE);
		if (nTemp < 1)
			nTemp = 1;
		else if (nTemp > 3600)
			nTemp = 3600;
		m_ssSettings.nDisplayStartTime = nTemp * 1000;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTOFFSETBEGIN, FALSE, FALSE);
		if (nTemp < 0)
			nTemp = 0;
		else if (nTemp > 3600)
			nTemp = 3600;
		m_ssSettings.nShowBeginOffset = nTemp * 1000;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTOFFSETEND, FALSE, FALSE);
		if (nTemp < 0)
			nTemp = 0;
		else if (nTemp > 3600)
			nTemp = 3600;
		m_ssSettings.nShowEndOffset = nTemp * 1000;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTDISABLESHORT, FALSE, FALSE);
		if (nTemp < 0)
			nTemp = 0;
		else if (nTemp > 3600)
			nTemp = 3600;
		m_ssSettings.nShortTrackLength = nTemp;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKDISABLESHORTTRACKS) == BST_CHECKED)
			m_ssSettings.bNoDisplayShortTracks = true;
		else
			m_ssSettings.bNoDisplayShortTracks = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKTOPMOST) == BST_CHECKED)
			m_ssSettings.bOSDTopMost = true;
		else
			m_ssSettings.bOSDTopMost = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKUSEFADE) == BST_CHECKED)
			m_ssSettings.bFadeEffects = true;
		else
			m_ssSettings.bFadeEffects = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_RDOALWAYSSHOW) == BST_CHECKED)
			m_ssSettings.bOSDAlwaysUp = true;
		else
			m_ssSettings.bOSDAlwaysUp = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKUSEVISUALS) == BST_CHECKED)
			m_ssSettings.bVisualFeedback = false;
		else
			m_ssSettings.bVisualFeedback = true;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKHIDEDISPLAYITUNES) == BST_CHECKED)
			m_ssSettings.bNoDisplayWhenActive = true;
		else
			m_ssSettings.bNoDisplayWhenActive = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATEND) == BST_CHECKED)
			m_ssSettings.bShowDisplayAtEnd = true;
		else
			m_ssSettings.bShowDisplayAtEnd = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWATSTART) == BST_CHECKED)
			m_ssSettings.bShowDisplayAtStart = true;
		else
			m_ssSettings.bShowDisplayAtStart = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKEXCEPTWHENPAUSED) == BST_CHECKED)
			m_ssSettings.bOSDHideWhenPaused = true;
		else
			m_ssSettings.bOSDHideWhenPaused = false;

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK LayoutSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetDlgItemText(hwndDlg, IDC_EDTOSDTEXT, m_ssSettings.wcOSDFormat);

		SendDlgItemMessage(hwndDlg, IDC_CMBJUSTIFY, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Left");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUSTIFY, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Center");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUSTIFY, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Right");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUSTIFY, CB_SETCURSEL, m_ssSettings.nOSDTextJustify, 0);

		SetDlgItemText(hwndDlg, IDC_EDTITUNESTITLE, m_ssSettings.wciTunesTitle);
		SetDlgItemText(hwndDlg, IDC_EDTCSIFORMAT, m_ssSettings.wcCSIFormat);

		if (m_ssSettings.bOSDShowMissingTags)
			CheckDlgButton(hwndDlg, IDC_CHKMISSINGTAGS, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKMISSINGTAGS, BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Album title [%album%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Artist name [%artist%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Bit rate [%bit_rate%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Comment [%comment%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Composer name [%composer%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Genre [%genre%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Rating [%rating%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Tags [%tags%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Total album tracks [%total_tracks%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Track length [%track_length%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Track number [%tracknumber%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Track position [%track_position%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Track title [%track%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Track year [%year%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Volume adjustment [%normalize%]");
		SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_SETCURSEL, 0, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMBPOSTEXT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Right of art");
		SendDlgItemMessage(hwndDlg, IDC_CMBPOSTEXT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Below art");
		SendDlgItemMessage(hwndDlg, IDC_CMBPOSTEXT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Above art");
		SendDlgItemMessage(hwndDlg, IDC_CMBPOSTEXT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Left of art");
		SendDlgItemMessage(hwndDlg, IDC_CMBPOSTEXT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Art only");
		SendDlgItemMessage(hwndDlg, IDC_CMBPOSTEXT, CB_SETCURSEL, m_ssSettings.nDisplayLayout, 0);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BTNINSERT:
		{
			int nPos = SendDlgItemMessage(hwndDlg, IDC_CMBCONSTANT, CB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDTOSDTEXT, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)wcConstants[nPos]);
			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		wchar_t wcTemp[512];

		GetDlgItemText(hwndDlg, IDC_EDTOSDTEXT, wcTemp, 512);
		wcscpy_s(m_ssSettings.wcOSDFormat, 512, wcTemp);

		m_ssSettings.nOSDTextJustify = SendDlgItemMessage(hwndDlg, IDC_CMBJUSTIFY, CB_GETCURSEL, 0, 0);

		m_ssSettings.nDisplayLayout = SendDlgItemMessage(hwndDlg, IDC_CMBPOSTEXT, CB_GETCURSEL, 0, 0);

		GetDlgItemText(hwndDlg, IDC_EDTITUNESTITLE, wcTemp, 64);
		wcscpy_s(m_ssSettings.wciTunesTitle, 64, wcTemp);

		GetDlgItemText(hwndDlg, IDC_EDTCSIFORMAT, wcTemp, 128);
		wcscpy_s(m_ssSettings.wcCSIFormat, 128, wcTemp);

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKMISSINGTAGS) == BST_CHECKED)
			m_ssSettings.bOSDShowMissingTags = true;
		else
			m_ssSettings.bOSDShowMissingTags = false;

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK ArtSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetDlgItemInt(hwndDlg, IDC_EDTARTCONST, m_ssSettings.nArtConstSize, FALSE);

		switch (m_ssSettings.nArtMode)
		{
		case 2: //scale to x pixels
		{
			CheckDlgButton(hwndDlg, IDC_RBARTMODE2, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RBARTMODE3, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), TRUE);
			break;
		}
		default: //scale to text height
		{
			CheckDlgButton(hwndDlg, IDC_RBARTMODE2, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RBARTMODE3, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), FALSE);
		}
		}

		if (m_ssSettings.bStaticArt)
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), TRUE);
			CheckDlgButton(hwndDlg, IDC_CHKSTATICART, BST_CHECKED);
		}
		else
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), FALSE);
			CheckDlgButton(hwndDlg, IDC_CHKSTATICART, BST_UNCHECKED);
		}

		if (m_ssSettings.bShowArtwork)
		{
			CheckDlgButton(hwndDlg, IDC_CHKSHOWART, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE2), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE3), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKUSEFOLDERART), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSTATICART), TRUE);
		}
		else
		{
			CheckDlgButton(hwndDlg, IDC_CHKSHOWART, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE2), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE3), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKUSEFOLDERART), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSTATICART), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), FALSE);
		}

		if (m_ssSettings.bFolderArt)
			CheckDlgButton(hwndDlg, IDC_CHKUSEFOLDERART, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKUSEFOLDERART, BST_UNCHECKED);

		SetDlgItemText(hwndDlg, IDC_EDTSTATICART, m_ssSettings.wcStaticArtPath);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_CHKSHOWART:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWART) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE2), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE3), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), TRUE);
				if (IsDlgButtonChecked(hwndDlg, IDC_CHKSTATICART) == BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), TRUE);
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), FALSE);
				}
				if (IsDlgButtonChecked(hwndDlg, IDC_RBARTMODE2) == BST_CHECKED)
				{
					CheckDlgButton(hwndDlg, IDC_RBARTMODE2, BST_CHECKED);
					CheckDlgButton(hwndDlg, IDC_RBARTMODE3, BST_UNCHECKED);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), TRUE);
					break;
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_RBARTMODE3) == BST_CHECKED)
				{
					CheckDlgButton(hwndDlg, IDC_RBARTMODE2, BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_RBARTMODE3, BST_CHECKED);
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), FALSE);
				}
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE2), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RBARTMODE3), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), FALSE);
			}
			break;
		}
		case IDC_CHKSTATICART:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_CHKSTATICART) == BST_CHECKED)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDTSTATICART), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSTATICART), FALSE);
			}
			break;
		}
		case IDC_BTNSTATICART:
		{
			OPENFILENAME ofn = { 0 };

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.exif;*.wmf;*.emf\0\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = m_ssSettings.wcStaticArtPath;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = _T("Select an image to be used to override album art for all tracks");
			ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

			if (TRUE == GetOpenFileName(&ofn))
			{
				SetDlgItemText(hwndDlg, IDC_EDTSTATICART, m_ssSettings.wcStaticArtPath);
			}
			break;
		}
		case IDC_RBARTMODE2:
		{
			CheckDlgButton(hwndDlg, IDC_RBARTMODE2, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RBARTMODE3, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), TRUE);
			break;
		}
		case IDC_RBARTMODE3:
		{
			CheckDlgButton(hwndDlg, IDC_RBARTMODE2, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RBARTMODE3, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTARTCONST), FALSE);
			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		int nTemp = 0;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTARTCONST, FALSE, FALSE);
		if (nTemp < 1)
			nTemp = 1;
		else if (nTemp > 500)
			nTemp = 500;
		m_ssSettings.nArtConstSize = nTemp;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWART) == BST_CHECKED)
			m_ssSettings.bShowArtwork = true;
		else
			m_ssSettings.bShowArtwork = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSTATICART) == BST_CHECKED)
			m_ssSettings.bStaticArt = true;
		else
			m_ssSettings.bStaticArt = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_RBARTMODE2) == BST_CHECKED)
			m_ssSettings.nArtMode = 2;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RBARTMODE3) == BST_CHECKED)
			m_ssSettings.nArtMode = 3;

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK MiscSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		if (m_ssSettings.bShowInTray)
			CheckDlgButton(hwndDlg, IDC_CHKSHOWINTRAY, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKSHOWINTRAY, BST_UNCHECKED);

		retVal = TRUE;
		break;
	}
	case WM_APPLY:
	{
		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSHOWINTRAY) == BST_CHECKED)
		{
			m_ssSettings.bShowInTray = true;
		}
		else
		{
			m_ssSettings.bShowInTray = false;

			// Only show the info if they changed from shown to not shown
			if (true == _ssOriginal.bShowInTray)
				MessageBox(hwndDlg, L"iTunesControl will no longer show up in the system tray. To access the settings dialog, you must use the \"Show Settings\" hotkey, which defaults to \"Win + Ctrl + Shift + S\".", L"Settings Change", MB_OK | MB_ICONINFORMATION);
		}

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

// Function LoadAnImage: accepts a file name and returns a HBITMAP.
// On error, it returns 0.
HBITMAP LoadAnImage(wchar_t* wcFilename)
{
	Gdiplus::Bitmap bmp(wcFilename);
	HBITMAP hbmp;
	bmp.GetHBITMAP(0, &hbmp);
	return hbmp;
}

INT_PTR CALLBACK RatingsSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		wchar_t wcTemp[2] = { 0 };

		Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDTFULLCHAR), 1);
		wcTemp[0] = m_ssSettings.wcRatingFullChar;
		SetDlgItemText(hwndDlg, IDC_EDTFULLCHAR, wcTemp);
		Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDTHALFCHAR), 1);
		wcTemp[0] = m_ssSettings.wcRatingHalfChar;
		SetDlgItemText(hwndDlg, IDC_EDTHALFCHAR, wcTemp);
		Edit_LimitText(GetDlgItem(hwndDlg, IDC_EDTEMPTYCHAR), 1);
		wcTemp[0] = m_ssSettings.wcRatingEmptyChar;
		SetDlgItemText(hwndDlg, IDC_EDTEMPTYCHAR, wcTemp);

		g_hbmFullStar = LoadAnImage(m_ssSettings.wcRatingFullPath);
		HBITMAP hBmp = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_STCFULLSTAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmFullStar);
		DeleteObject((HGDIOBJ)hBmp);

		g_hbmHalfStar = LoadAnImage(m_ssSettings.wcRatingHalfPath);
		hBmp = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_STCHALFSTAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmHalfStar);
		DeleteObject((HGDIOBJ)hBmp);

		g_hbmEmptyStar = LoadAnImage(m_ssSettings.wcRatingEmptyPath);
		hBmp = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_STCEMPTYSTAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmEmptyStar);
		DeleteObject((HGDIOBJ)hBmp);

		g_hbrShapeFill = CreateSolidBrush(m_ssSettings.lRatingShapeFill);
		g_hbrShapeOutline = CreateSolidBrush(m_ssSettings.lRatingShapeOutline);
		SetDlgItemText(hwndDlg, IDC_STCSHAPEOUTLINE, L"");
		SetDlgItemText(hwndDlg, IDC_STCSHAPEFILL, L"");

		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Circle");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Rectangle");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Star");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Hexagon");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPETYPE, CB_SETCURSEL, m_ssSettings.nRatingShapeType, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPESTYLE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Left to Right");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPESTYLE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Right to Left");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPESTYLE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Top to Bottom");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPESTYLE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Bottom to Top");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHAPESTYLE, CB_SETCURSEL, m_ssSettings.nRatingShapeStyle, 0);

		switch (m_ssSettings.nRatingType)
		{
		case 0: // text ratings
		{
			CheckDlgButton(hwndDlg, IDC_RDOTEXT, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOIMAGE, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHAPES, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTFULLCHAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTHALFCHAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTEMPTYCHAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNFULLSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNHALFSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEMPTYSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPETYPE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPESTYLE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEFILL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEOUTLINE), FALSE);
			break;
		}
		case 1: // image ratings
		{
			CheckDlgButton(hwndDlg, IDC_RDOTEXT, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOIMAGE, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHAPES, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTFULLCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTHALFCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTEMPTYCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNFULLSTAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNHALFSTAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEMPTYSTAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPETYPE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPESTYLE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEFILL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEOUTLINE), FALSE);
			break;
		}
		case 2: // shape ratings
		{
			CheckDlgButton(hwndDlg, IDC_RDOTEXT, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOIMAGE, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHAPES, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTFULLCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTHALFCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTEMPTYCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNFULLSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNHALFSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEMPTYSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPETYPE), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPESTYLE), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEFILL), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEOUTLINE), TRUE);
			break;
		}
		}

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BTNFULLSTAR:
		{
			OPENFILENAME ofn = { 0 };

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.exif;*.wmf;*.emf\0\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = m_ssSettings.wcRatingFullPath;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = _T("Select an image to be used as a full rating star");
			ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

			if (TRUE == GetOpenFileName(&ofn))
			{
				DeleteObject((HGDIOBJ)g_hbmFullStar);
				g_hbmFullStar = LoadAnImage(m_ssSettings.wcRatingFullPath);
				HBITMAP hBmp = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_STCFULLSTAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmFullStar);
				DeleteObject((HGDIOBJ)hBmp);
			}

			break;
		}
		case IDC_BTNHALFSTAR:
		{
			OPENFILENAME ofn = { 0 };

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.exif;*.wmf;*.emf\0\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = m_ssSettings.wcRatingHalfPath;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = _T("Select an image to be used as a half rating star");
			ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

			if (TRUE == GetOpenFileName(&ofn))
			{
				DeleteObject((HGDIOBJ)g_hbmHalfStar);
				g_hbmHalfStar = LoadAnImage(m_ssSettings.wcRatingHalfPath);
				HBITMAP hBmp = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_STCHALFSTAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmHalfStar);
				DeleteObject((HGDIOBJ)hBmp);
			}

			break;
		}
		case IDC_BTNEMPTYSTAR:
		{
			OPENFILENAME ofn = { 0 };

			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.exif;*.wmf;*.emf\0\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = m_ssSettings.wcRatingEmptyPath;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = _T("Select an image to be used as an empty rating star");
			ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

			if (TRUE == GetOpenFileName(&ofn))
			{
				DeleteObject((HGDIOBJ)g_hbmEmptyStar);
				g_hbmEmptyStar = LoadAnImage(m_ssSettings.wcRatingEmptyPath);
				HBITMAP hBmp = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_STCEMPTYSTAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmEmptyStar);
				DeleteObject((HGDIOBJ)hBmp);
			}

			break;
		}
		case IDC_STCSHAPEFILL:
		{
			if (HIWORD(wParam) == STN_CLICKED)
			{
				CHOOSECOLOR cc;
				COLORREF * CustColors = new COLORREF[16];

				CustColors[0] = m_ssSettings.lRatingShapeFill;

				ZeroMemory(&cc, sizeof(cc));
				cc.lStructSize = sizeof(cc);
				cc.rgbResult = m_ssSettings.lRatingShapeFill;
				cc.hwndOwner = hwndDlg;
				cc.lpCustColors = CustColors;
				cc.Flags = CC_ANYCOLOR | CC_RGBINIT;

				if (ChooseColor(&cc) == TRUE)
				{
					m_ssSettings.lRatingShapeFill = cc.rgbResult;
					DeleteObject((HGDIOBJ)g_hbrShapeFill);
					g_hbrShapeFill = CreateSolidBrush(m_ssSettings.lRatingShapeFill);
					RedrawWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEFILL), NULL, NULL, RDW_INVALIDATE);
				}

				delete[] CustColors;
			}

			break;
		}
		case IDC_STCSHAPEOUTLINE:
		{
			if (HIWORD(wParam) == STN_CLICKED)
			{
				CHOOSECOLOR cc;
				COLORREF * CustColors = new COLORREF[16];

				CustColors[0] = m_ssSettings.lRatingShapeOutline;

				ZeroMemory(&cc, sizeof(cc));
				cc.lStructSize = sizeof(cc);
				cc.rgbResult = m_ssSettings.lRatingShapeOutline;
				cc.hwndOwner = hwndDlg;
				cc.lpCustColors = CustColors;
				cc.Flags = CC_ANYCOLOR | CC_RGBINIT;

				if (ChooseColor(&cc) == TRUE)
				{
					m_ssSettings.lRatingShapeOutline = cc.rgbResult;
					DeleteObject((HGDIOBJ)g_hbrShapeOutline);
					g_hbrShapeOutline = CreateSolidBrush(m_ssSettings.lRatingShapeOutline);
					RedrawWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEOUTLINE), NULL, NULL, RDW_INVALIDATE);
				}

				delete[] CustColors;
			}

			break;
		}
		case IDC_RDOTEXT:
		{
			CheckDlgButton(hwndDlg, IDC_RDOTEXT, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOIMAGE, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHAPES, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTFULLCHAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTHALFCHAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTEMPTYCHAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNFULLSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNHALFSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEMPTYSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPETYPE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPESTYLE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEFILL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEOUTLINE), FALSE);
			break;
		}
		case IDC_RDOIMAGE:
		{
			CheckDlgButton(hwndDlg, IDC_RDOTEXT, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOIMAGE, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHAPES, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTFULLCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTHALFCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTEMPTYCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNFULLSTAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNHALFSTAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEMPTYSTAR), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPETYPE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPESTYLE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEFILL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEOUTLINE), FALSE);
			break;
		}
		case IDC_RDOSHAPES:
		{
			CheckDlgButton(hwndDlg, IDC_RDOTEXT, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOIMAGE, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSHAPES, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTFULLCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTHALFCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDTEMPTYCHAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNFULLSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNHALFSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTNEMPTYSTAR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPETYPE), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMBSHAPESTYLE), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEFILL), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STCSHAPEOUTLINE), TRUE);
			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		wchar_t wcTemp[2] = { 0 };

		if (IsDlgButtonChecked(hwndDlg, IDC_RDOTEXT) == BST_CHECKED)
			m_ssSettings.nRatingType = 0;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RDOIMAGE) == BST_CHECKED)
			m_ssSettings.nRatingType = 1;
		else if (IsDlgButtonChecked(hwndDlg, IDC_RDOSHAPES) == BST_CHECKED)
			m_ssSettings.nRatingType = 2;

		m_ssSettings.nRatingShapeType = SendDlgItemMessage(hwndDlg, IDC_CMBSHAPETYPE, CB_GETCURSEL, 0, 0);
		m_ssSettings.nRatingShapeStyle = SendDlgItemMessage(hwndDlg, IDC_CMBSHAPESTYLE, CB_GETCURSEL, 0, 0);

		GetDlgItemText(hwndDlg, IDC_EDTFULLCHAR, wcTemp, 2);
		m_ssSettings.wcRatingFullChar = wcTemp[0];
		GetDlgItemText(hwndDlg, IDC_EDTHALFCHAR, wcTemp, 2);
		m_ssSettings.wcRatingHalfChar = wcTemp[0];
		GetDlgItemText(hwndDlg, IDC_EDTEMPTYCHAR, wcTemp, 2);
		m_ssSettings.wcRatingEmptyChar = wcTemp[0];

		return TRUE;
		break;
	}
	case WM_CTLCOLORSTATIC:
	{
		int nCntrlID = GetDlgCtrlID((HWND)lParam);

		switch (nCntrlID)
		{
		case IDC_STCSHAPEFILL:
			return (LRESULT)g_hbrShapeFill;
		case IDC_STCSHAPEOUTLINE:
			return (LRESULT)g_hbrShapeOutline;
		}

		return NULL;
	}
	case WM_CLOSE:
	{
		retVal = 0;
		DeleteObject((HGDIOBJ)g_hbrShapeFill);
		DeleteObject((HGDIOBJ)g_hbrShapeOutline);
		DeleteObject((HGDIOBJ)g_hbmFullStar);
		DeleteObject((HGDIOBJ)g_hbmHalfStar);
		DeleteObject((HGDIOBJ)g_hbmEmptyStar);
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK AdvancedSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		if (m_ssSettings.bShowCfgSaveWarning)
			CheckDlgButton(hwndDlg, IDC_CHKWARNSAVECHGS, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKWARNSAVECHGS, BST_UNCHECKED);

		if (m_ssSettings.bDisableACOnFocus)
			CheckDlgButton(hwndDlg, IDC_CHKIGNOREACONFOCUS, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKIGNOREACONFOCUS, BST_UNCHECKED);

		if (m_ssSettings.bAllowBlankLines)
			CheckDlgButton(hwndDlg, IDC_CHKALLOWBLANKS, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKALLOWBLANKS, BST_UNCHECKED);

		if (m_ssSettings.bOnlyToggleRepeatOne)
			CheckDlgButton(hwndDlg, IDC_CHKONLYTOGGLEREPEATONE, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKONLYTOGGLEREPEATONE, BST_UNCHECKED);

		wchar_t tmp[10];
		swprintf_s(tmp, L"%.2f", m_ssSettings.fDropShadowOffset);
		SetDlgItemText(hwndDlg, IDC_EDTDROPSHADOWOFFSET, tmp);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BTNOPENCFGFLDR:
			wchar_t szPath[MAX_PATH];

			if (S_OK == SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA, NULL, 0, L"iTunesControl", szPath))
				ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);
			else if (IsPortable())
			{
				GetModuleFileName(NULL, szPath, MAX_PATH);
				PathRemoveFileSpec(szPath);
				ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);
			}
			else
				MessageBox(hwndDlg, L"An error was encountered while opening the configuration folder.", L"Error", MB_OK);

			break;
		}
		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		if (IsDlgButtonChecked(hwndDlg, IDC_CHKWARNSAVECHGS) == BST_CHECKED)
			m_ssSettings.bShowCfgSaveWarning = true;
		else
			m_ssSettings.bShowCfgSaveWarning = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKALLOWBLANKS) == BST_CHECKED)
			m_ssSettings.bAllowBlankLines = true;
		else
			m_ssSettings.bAllowBlankLines = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKIGNOREACONFOCUS) == BST_CHECKED)
			m_ssSettings.bDisableACOnFocus = true;
		else
			m_ssSettings.bDisableACOnFocus = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKONLYTOGGLEREPEATONE) == BST_CHECKED)
			m_ssSettings.bOnlyToggleRepeatOne = true;
		else
			m_ssSettings.bOnlyToggleRepeatOne = false;

		wchar_t tmp[10];
		GetDlgItemText(hwndDlg, IDC_EDTDROPSHADOWOFFSET, tmp, 10);
		float ftmp = (float)wcstod(tmp, NULL);
		if (ftmp > 10)
			ftmp = 10;
		else if (ftmp < 0.1f)
			ftmp = 0.1f;
		m_ssSettings.fDropShadowOffset = ftmp;

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK SongSearchSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetDlgItemInt(hwndDlg, IDC_EDTMAXSSRESULTS, m_ssSettings.nMaxSongSearchResults, FALSE);

		if (m_ssSettings.bAlwaysSearchLibrary)
		{
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHCURRENT, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHLIBRARY, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHCURRENT, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHLIBRARY, BST_UNCHECKED);
		}

		SendDlgItemMessage(hwndDlg, IDC_CMBENTERACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Play in Current Location");
		SendDlgItemMessage(hwndDlg, IDC_CMBENTERACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Play in Current Playlist");
		SendDlgItemMessage(hwndDlg, IDC_CMBENTERACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Enqueue in iTunes DJ");
		SendDlgItemMessage(hwndDlg, IDC_CMBENTERACTION, CB_SETCURSEL, m_ssSettings.nSSEnterAction, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMBSHIFTACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Play in Current Location");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHIFTACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Play in Current Playlist");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHIFTACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Enqueue in iTunes DJ");
		SendDlgItemMessage(hwndDlg, IDC_CMBSHIFTACTION, CB_SETCURSEL, m_ssSettings.nSSShiftEnterAction, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMBCTRLACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Play in Current Location");
		SendDlgItemMessage(hwndDlg, IDC_CMBCTRLACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Play in Current Playlist");
		SendDlgItemMessage(hwndDlg, IDC_CMBCTRLACTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Enqueue in iTunes DJ");
		SendDlgItemMessage(hwndDlg, IDC_CMBCTRLACTION, CB_SETCURSEL, m_ssSettings.nSSCtrlEnterAction, 0);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_RDOSEARCHCURRENT:
		{
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHLIBRARY, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHCURRENT, BST_CHECKED);
			break;
		}
		case IDC_RDOSEARCHLIBRARY:
		{
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHLIBRARY, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RDOSEARCHCURRENT, BST_UNCHECKED);
			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		m_ssSettings.nMaxSongSearchResults = GetDlgItemInt(hwndDlg, IDC_EDTMAXSSRESULTS, FALSE, FALSE);

		if (IsDlgButtonChecked(hwndDlg, IDC_RDOSEARCHCURRENT) == BST_CHECKED)
			m_ssSettings.bAlwaysSearchLibrary = false;
		else
			m_ssSettings.bAlwaysSearchLibrary = true;

		m_ssSettings.nSSEnterAction = SendDlgItemMessage(hwndDlg, IDC_CMBENTERACTION, CB_GETCURSEL, 0, 0);
		m_ssSettings.nSSShiftEnterAction = SendDlgItemMessage(hwndDlg, IDC_CMBSHIFTACTION, CB_GETCURSEL, 0, 0);
		m_ssSettings.nSSCtrlEnterAction = SendDlgItemMessage(hwndDlg, IDC_CMBCTRLACTION, CB_GETCURSEL, 0, 0);

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK FeaturesSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetDlgItemInt(hwndDlg, IDC_EDTJUMPONEVALUE, m_ssSettings.nJumpOneValue, FALSE);
		SetDlgItemInt(hwndDlg, IDC_EDTJUMPTWOVALUE, m_ssSettings.nJumpTwoValue, FALSE);
		SetDlgItemText(hwndDlg, IDC_EDTQUICKPLAYLIST, m_ssSettings.wcQuickPlaylist);

		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPONETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Seconds (Absolute)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPONETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Seconds (Relative)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPONETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Percent (Absolute)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPONETYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Percent (Relative)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPONETYPE, CB_SETCURSEL, m_ssSettings.nJumpOneType, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPTWOTYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Seconds (Absolute)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPTWOTYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Seconds (Relative)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPTWOTYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Percent (Absolute)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPTWOTYPE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"Percent (Relative)");
		SendDlgItemMessage(hwndDlg, IDC_CMBJUMPTWOTYPE, CB_SETCURSEL, m_ssSettings.nJumpTwoType, 0);

		if (m_ssSettings.nTagField == 1)
			CheckDlgButton(hwndDlg, IDC_CHKGROUPINGTAG, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKGROUPINGTAG, BST_UNCHECKED);

		if (m_ssSettings.bDeleteFromDisk)
			CheckDlgButton(hwndDlg, IDC_CHKDELETEFROMDISK, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKDELETEFROMDISK, BST_UNCHECKED);

		if (m_ssSettings.bPromptOnDelete)
			CheckDlgButton(hwndDlg, IDC_CHKPROMPTTODELETE, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKPROMPTTODELETE, BST_UNCHECKED);

		SetDlgItemInt(hwndDlg, IDC_EDTVOLCHGINT, m_ssSettings.nVolumeChgInt, FALSE);

		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"½ Star");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"1 Star");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"1½ Stars");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"2 Stars");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"2½ Stars");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"3 Stars");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"3½ Stars");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"4 Stars");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"4½ Stars");
		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)L"5 Stars");

		SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_SETCURSEL, (m_ssSettings.nRatingDelta - 10) / 10, 0);

		retVal = TRUE;
		break;
	}
	case WM_APPLY:
	{
		int nTemp = 0;
		wchar_t wcTemp[64];

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTVOLCHGINT, FALSE, FALSE);
		if (nTemp < 1)
			nTemp = 1;
		else if (nTemp > 25)
			nTemp = 25;
		m_ssSettings.nVolumeChgInt = nTemp;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTJUMPONEVALUE, FALSE, TRUE);
		m_ssSettings.nJumpOneValue = nTemp;

		nTemp = GetDlgItemInt(hwndDlg, IDC_EDTJUMPTWOVALUE, FALSE, TRUE);
		m_ssSettings.nJumpTwoValue = nTemp;

		nTemp = SendDlgItemMessage(hwndDlg, IDC_CMBJUMPONETYPE, CB_GETCURSEL, 0, 0);
		m_ssSettings.nJumpOneType = nTemp;

		nTemp = SendDlgItemMessage(hwndDlg, IDC_CMBJUMPTWOTYPE, CB_GETCURSEL, 0, 0);
		m_ssSettings.nJumpTwoType = nTemp;

		nTemp = SendDlgItemMessage(hwndDlg, IDC_CMBRATINGDELTA, CB_GETCURSEL, 0, 0);
		m_ssSettings.nRatingDelta = 10 * nTemp + 10;

		GetDlgItemText(hwndDlg, IDC_EDTQUICKPLAYLIST, wcTemp, 64);
		wcscpy_s(m_ssSettings.wcQuickPlaylist, 64, wcTemp);

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKGROUPINGTAG) == BST_CHECKED)
			m_ssSettings.nTagField = 1;
		else
			m_ssSettings.nTagField = 0;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKDELETEFROMDISK) == BST_CHECKED)
			m_ssSettings.bDeleteFromDisk = true;
		else
			m_ssSettings.bDeleteFromDisk = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKPROMPTTODELETE) == BST_CHECKED)
			m_ssSettings.bPromptOnDelete = true;
		else
			m_ssSettings.bPromptOnDelete = false;

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

INT_PTR CALLBACK StartShutSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	int retVal = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		if (m_ssSettings.bPromptQuitiTunes)
			CheckDlgButton(hwndDlg, IDC_CHKPROMPTQUIT, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKPROMPTQUIT, BST_UNCHECKED);

		if (m_ssSettings.bMinimizeiTunesAtStart)
			CheckDlgButton(hwndDlg, IDC_CHKMINIMIZEATSTART, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKMINIMIZEATSTART, BST_UNCHECKED);

		if (m_ssSettings.bQuitiTunesOnExit)
		{
			CheckDlgButton(hwndDlg, IDC_CHKQUITITUNES, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKPROMPTQUIT), TRUE);
		}
		else
		{
			CheckDlgButton(hwndDlg, IDC_CHKQUITITUNES, BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKPROMPTQUIT), FALSE);
		}

		if (m_ssSettings.bStartiTunesAtStart)
			CheckDlgButton(hwndDlg, IDC_CHKSTARTITUNES, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKSTARTITUNES, BST_UNCHECKED);

		if (m_ssSettings.bQuitWithiTunes)
			CheckDlgButton(hwndDlg, IDC_CHKQUITWITHITUNES, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKQUITWITHITUNES, BST_UNCHECKED);

		if (m_ssSettings.bStartWithiTunes)
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWITHITUNES, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWITHITUNES, BST_UNCHECKED);

		if (m_ssSettings.bStartWithWindows)
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWINDOWS, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWINDOWS, BST_UNCHECKED);

		if (IsPortable())
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSTARTWINDOWS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSTARTWITHITUNES), FALSE);
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWITHITUNES, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWINDOWS, BST_UNCHECKED);
		}

		if (m_ssSettings.bStartOnHotkeys)
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWITHHOTKEYS, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHKSTARTWITHHOTKEYS, BST_UNCHECKED);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_CHKQUITITUNES:
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_CHKQUITITUNES) == BST_CHECKED)
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKPROMPTQUIT), TRUE);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHKPROMPTQUIT), FALSE);

			break;
		}
		}

		retVal = 0;
		break;
	}
	case WM_APPLY:
	{
		if (IsDlgButtonChecked(hwndDlg, IDC_CHKPROMPTQUIT) == BST_CHECKED)
			m_ssSettings.bPromptQuitiTunes = true;
		else
			m_ssSettings.bPromptQuitiTunes = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKQUITITUNES) == BST_CHECKED)
			m_ssSettings.bQuitiTunesOnExit = true;
		else
			m_ssSettings.bQuitiTunesOnExit = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSTARTITUNES) == BST_CHECKED)
			m_ssSettings.bStartiTunesAtStart = true;
		else
			m_ssSettings.bStartiTunesAtStart = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSTARTWINDOWS) == BST_CHECKED)
			m_ssSettings.bStartWithWindows = true;
		else
			m_ssSettings.bStartWithWindows = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKQUITWITHITUNES) == BST_CHECKED)
			m_ssSettings.bQuitWithiTunes = true;
		else
			m_ssSettings.bQuitWithiTunes = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSTARTWITHITUNES) == BST_CHECKED)
			m_ssSettings.bStartWithiTunes = true;
		else
			m_ssSettings.bStartWithiTunes = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKSTARTWITHHOTKEYS) == BST_CHECKED)
			m_ssSettings.bStartOnHotkeys = true;
		else
			m_ssSettings.bStartOnHotkeys = false;

		if (IsDlgButtonChecked(hwndDlg, IDC_CHKMINIMIZEATSTART) == BST_CHECKED)
			m_ssSettings.bMinimizeiTunesAtStart = true;
		else
			m_ssSettings.bMinimizeiTunesAtStart = false;

		return TRUE;
		break;
	}

	case WM_CLOSE:
	{
		retVal = 0;
		break;
	}
	}

	return retVal;
}

bool MakeHotkeyText(sHotkey shkHotkey, wchar_t *wcBuffer)
{
	bool bValid = false;

	if ((shkHotkey.bWin || shkHotkey.bShift || shkHotkey.bAlt || shkHotkey.bControl || shkHotkey.bSingle) && wcscmp(shkHotkey.wcKey, L"") != 0)
	{
		bValid = true;
		wsprintf(wcBuffer, L"%s%s%s%s%s",
			(shkHotkey.bWin ? L"Win + " : L""),
			(shkHotkey.bControl ? L"Ctrl + " : L""),
			(shkHotkey.bShift ? L"Shift + " : L""),
			(shkHotkey.bAlt ? L"Alt + " : L""),
			shkHotkey.wcKey);
	}
	else
	{
		bValid = false;
		wsprintf(wcBuffer, L"(None)");
	}

	return bValid;
}

bool LoadHotkey(HotKeyItem hkiHotkey, sHotkey *shkHotkey, wchar_t *wcBuffer)
{
	UINT key = hkiHotkey.uiKey;
	UINT mod = hkiHotkey.uiMods;

	if (mod & MOD_SHIFT)
		shkHotkey->bShift = true;
	if (mod & MOD_CONTROL)
		shkHotkey->bControl = true;
	if (mod & MOD_WIN)
		shkHotkey->bWin = true;
	if (mod & MOD_ALT)
		shkHotkey->bAlt = true;

	if (mod == MOD_APPCOMMAND)
		return ValidateAppCmd(key, wcBuffer, shkHotkey);
	else
		return ValidateHotkey(key, MapVirtualKey(key, MAPVK_VK_TO_VSC) << 16 | 1 << 24, wcBuffer, shkHotkey);
}

bool ValidateAppCmd(WPARAM wParam, wchar_t *wcBuffer, sHotkey *shkHotkey)
{
	// We're always going to return valid, and these will always be single
	shkHotkey->bSingle = true;
	shkHotkey->uiKey = wParam;
	shkHotkey->bAppCmd = true;

	// 30 by name + unknown catch-all
	if (wParam == APPCOMMAND_LAUNCH_MEDIA_SELECT) wcscpy_s(shkHotkey->wcKey, L"Media Select (AC)");
	else if (wParam == APPCOMMAND_MEDIA_CHANNEL_DOWN) wcscpy_s(shkHotkey->wcKey, L"Channel Down (AC)");
	else if (wParam == APPCOMMAND_MEDIA_CHANNEL_UP) wcscpy_s(shkHotkey->wcKey, L"Channel Up (AC)");
	else if (wParam == APPCOMMAND_MEDIA_FAST_FORWARD) wcscpy_s(shkHotkey->wcKey, L"Fast Forward (AC)");
	else if (wParam == APPCOMMAND_MEDIA_NEXTTRACK) wcscpy_s(shkHotkey->wcKey, L"Next Track (AC)");
	else if (wParam == APPCOMMAND_MEDIA_PAUSE) wcscpy_s(shkHotkey->wcKey, L"Pause (AC)");
	else if (wParam == APPCOMMAND_MEDIA_PLAY) wcscpy_s(shkHotkey->wcKey, L"Play (AC)");
	else if (wParam == APPCOMMAND_MEDIA_PLAY_PAUSE) wcscpy_s(shkHotkey->wcKey, L"Play / Pause (AC)");
	else if (wParam == APPCOMMAND_MEDIA_PREVIOUSTRACK) wcscpy_s(shkHotkey->wcKey, L"Previous Track (AC)");
	else if (wParam == APPCOMMAND_MEDIA_RECORD) wcscpy_s(shkHotkey->wcKey, L"Record (AC)");
	else if (wParam == APPCOMMAND_MEDIA_REWIND) wcscpy_s(shkHotkey->wcKey, L"Rewind (AC)");
	else if (wParam == APPCOMMAND_MEDIA_STOP) wcscpy_s(shkHotkey->wcKey, L"Stop (AC)");
	else if (wParam == APPCOMMAND_VOLUME_DOWN) wcscpy_s(shkHotkey->wcKey, L"Volume Down (AC)");
	else if (wParam == APPCOMMAND_VOLUME_MUTE) wcscpy_s(shkHotkey->wcKey, L"Volume Mute (AC)");
	else if (wParam == APPCOMMAND_VOLUME_UP) wcscpy_s(shkHotkey->wcKey, L"Volume Up (AC)");
	else if (wParam == APPCOMMAND_BASS_BOOST) wcscpy_s(shkHotkey->wcKey, L"Bass Boost (AC)");
	else if (wParam == APPCOMMAND_BASS_DOWN) wcscpy_s(shkHotkey->wcKey, L"Bass Down (AC)");
	else if (wParam == APPCOMMAND_BASS_UP) wcscpy_s(shkHotkey->wcKey, L"Bass Up (AC)");
	else if (wParam == APPCOMMAND_BROWSER_BACKWARD) wcscpy_s(shkHotkey->wcKey, L"Browser Back (AC)");
	else if (wParam == APPCOMMAND_BROWSER_FAVORITES) wcscpy_s(shkHotkey->wcKey, L"Browser Favorites (AC)");
	else if (wParam == APPCOMMAND_BROWSER_FORWARD) wcscpy_s(shkHotkey->wcKey, L"Browser Forward (AC)");
	else if (wParam == APPCOMMAND_BROWSER_HOME) wcscpy_s(shkHotkey->wcKey, L"Browser Home (AC)");
	else if (wParam == APPCOMMAND_BROWSER_REFRESH) wcscpy_s(shkHotkey->wcKey, L"Browser Refresh (AC)");
	else if (wParam == APPCOMMAND_BROWSER_SEARCH) wcscpy_s(shkHotkey->wcKey, L"Browser Search (AC)");
	else if (wParam == APPCOMMAND_BROWSER_STOP) wcscpy_s(shkHotkey->wcKey, L"Browser Stop (AC)");
	else if (wParam == APPCOMMAND_MICROPHONE_VOLUME_DOWN) wcscpy_s(shkHotkey->wcKey, L"Mic Volume Down (AC)");
	else if (wParam == APPCOMMAND_MICROPHONE_VOLUME_MUTE) wcscpy_s(shkHotkey->wcKey, L"Mic Volume Mute (AC)");
	else if (wParam == APPCOMMAND_MICROPHONE_VOLUME_UP) wcscpy_s(shkHotkey->wcKey, L"Mic Volume Up (AC)");
	else if (wParam == APPCOMMAND_TREBLE_DOWN) wcscpy_s(shkHotkey->wcKey, L"Treble Down (AC)");
	else if (wParam == APPCOMMAND_TREBLE_UP) wcscpy_s(shkHotkey->wcKey, L"Treble Up (AC)");
	else wcscpy_s(shkHotkey->wcKey, L"<Unknown> (AC)");

	wcscpy_s(wcBuffer, 64, shkHotkey->wcKey);

	return true;
}

bool ValidateHotkey(WPARAM wParam, LPARAM lParam, wchar_t *wcBuffer, sHotkey *shkHotkey)
{
	int nNumber = 0;
	bool bValid = false;

	UINT uiCode = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
	if (VK_BACK == wParam)
	{
		memset((void *)shkHotkey, 0, sizeof(sHotkey));
	}
	else if (VK_PAUSE == wParam || VK_CANCEL == wParam)
	{
		wcscpy_s(shkHotkey->wcKey, L"Pause / Break");
		shkHotkey->uiKey = wParam;
		shkHotkey->bSingle = true;
	}
	else if (0 != uiCode && wParam != VK_SPACE && wParam != VK_TAB && wParam != VK_RETURN)
	{
		wcscpy_s(shkHotkey->wcKey, (wchar_t*)&uiCode);
		shkHotkey->uiKey = wParam;
	}
	else if (wParam >= VK_LEFT && wParam <= VK_DOWN)
	{
		GetKeyNameText(lParam, shkHotkey->wcKey, 24);
		shkHotkey->uiKey = wParam;
	}
	else if (wParam == VK_PRIOR || wParam == VK_NEXT)
	{
		GetKeyNameText(lParam, shkHotkey->wcKey, 24);
		shkHotkey->uiKey = wParam;
		shkHotkey->bSingle = true;
	}
	else if (wParam == VK_END && wParam == VK_HOME)
	{
		GetKeyNameText(lParam, shkHotkey->wcKey, 24);
		shkHotkey->uiKey = wParam;
	}
	else if (wParam >= VK_INSERT && wParam <= VK_DELETE)
	{
		GetKeyNameText(lParam, shkHotkey->wcKey, 24);
		shkHotkey->uiKey = wParam;
	}
	else if (wParam == VK_SPACE)
	{
		wcscpy_s(shkHotkey->wcKey, L"Space");
		shkHotkey->uiKey = wParam;
	}
	else if (wParam == VK_TAB)
	{
		wcscpy_s(shkHotkey->wcKey, L"Tab");
		shkHotkey->uiKey = wParam;
	}
	else if (wParam == VK_RETURN)
	{
		wcscpy_s(shkHotkey->wcKey, L"Enter");
		shkHotkey->uiKey = wParam;
	}
	else if (wParam == VK_CLEAR)
	{
		wcscpy_s(shkHotkey->wcKey, L"Clear");
		shkHotkey->uiKey = wParam;
	}
	else if (wParam >= VK_F1 && wParam <= VK_F24)
	{
		nNumber = wParam - VK_F1 + 1;
		swprintf_s(shkHotkey->wcKey, 24, L"F%d", nNumber);
		shkHotkey->uiKey = wParam;
		shkHotkey->bSingle = true;
	}
	else if (wParam == VK_LWIN || wParam == VK_RWIN)
	{
		shkHotkey->bWin = true;
	}
	else if (wParam == VK_SHIFT)
	{
		shkHotkey->bShift = true;
	}
	else if (wParam == VK_CONTROL)
	{
		shkHotkey->bControl = true;
	}
	else if (VK_MENU == wParam)
	{
		shkHotkey->bAlt = true;
	}
	else if (wParam >= VK_BROWSER_BACK && wParam <= VK_LAUNCH_APP2)
	{
		nNumber = wParam - VK_BROWSER_BACK;
		wcscpy_s(shkHotkey->wcKey, 24, wcExtraKeys[nNumber]);
		shkHotkey->uiKey = wParam;
		shkHotkey->bSingle = true;
	}
	else
	{
		wcscpy_s(shkHotkey->wcKey, 24, L"<Unknown>");
		shkHotkey->uiKey = wParam;
	}

	bValid = MakeHotkeyText(*shkHotkey, wcBuffer);

	return bValid;
}

INT_PTR CALLBACK HotkeysSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int retVal = FALSE;

	if (HWM_HOOKMSG == uMsg)
	{
		wchar_t wcBuffer[64] = { 0 };
		sHotkey shkTemp;
		HotKeyItem hkiTemp = { 0 };

		hkiTemp.uiMods = wParam;
		hkiTemp.uiKey = (UINT16)lParam;

		memset(&shkTemp, NULL, sizeof(sHotkey));
		LoadHotkey(hkiTemp, &shkTemp, wcBuffer);
		memcpy((void*)&shkHold, (void*)&shkTemp, sizeof(sHotkey));

		SetDlgItemText(hwndDlg, IDC_EDTHOTKEY, wcBuffer);
		PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_EDTHOTKEY, EN_CHANGE), 0);

		int iItem = SendMessage(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), LVM_GETNEXTITEM, -1, LVNI_ALL | LVNI_SELECTED);
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = iItem;
		lvi.iSubItem = 1;
		lvi.pszText = wcBuffer;
		ListView_SetItem(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), &lvi);

		vector<HotKeyItem>::iterator p = vKeys->begin();
		int i = 0;
		while (p != vKeys->end() && hkiTemp.uiKey != 0)
		{
			if (i != iItem && hkiTemp.uiKey == p->uiKey && hkiTemp.uiMods == p->uiMods)
			{
				wchar_t wcMsg[100];
				swprintf_s(wcMsg, L"\"%s\" is already set as the hotkey for \"%s\"!", wcBuffer, p->wcPrintName);
				// If we let KillFocus unregister the hook, we wouldn't catch the key up events
				g_bKeepHookRegistered = true;
				MessageBox(hwndDlg, wcMsg, L"Hotkey Conflict", MB_OK);
				g_bKeepHookRegistered = false;
			}
			++i;
			++p;
		}

		return TRUE;
	}

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		SetWindowSubclass(GetDlgItem(hwndDlg, IDC_EDTHOTKEY), HotkeyEditSubclassProc, 0, NULL);
		hwndHotkeyDlg = hwndDlg;

		SetMatchMode(false);

		// Set up hotkey list view
		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_ORDER;
		// Action column
		lvc.fmt = LVCFMT_LEFT;
		lvc.iOrder = 0;
		lvc.pszText = _T("Action");
		lvc.cchTextMax = _countof(_T("Action"));
		ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), 0, &lvc);
		// Hotkey column
		lvc.iOrder = 1;
		lvc.pszText = _T("Hotkey");
		lvc.cchTextMax = _countof(_T("Hotkey"));
		ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), 1, &lvc);
		// Autosize columns

		DWORD dwStyle = SendDlgItemMessage(hwndDlg, IDC_LSTHOTKEYS, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
		dwStyle |= LVS_EX_FULLROWSELECT;
		SendDlgItemMessage(hwndDlg, IDC_LSTHOTKEYS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);

		SetDlgItemText(hwndDlg, IDC_EDTHOTKEY, L"(None)");
		m_bHotKeyChanged = false;
		m_lHotKeyOldIndex = -1;

		vector<HotKeyItem>::iterator p = vKeys->begin();

		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		wchar_t wcBuffer[64] = { 0 };
		sHotkey shkTemp;

		while (p != vKeys->end())
		{
			memset(&shkTemp, NULL, sizeof(sHotkey));

			lvi.iItem = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS));
			lvi.iSubItem = 0;
			lvi.pszText = p->wcPrintName;
			ListView_InsertItem(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), &lvi);

			LoadHotkey(*p, &shkTemp, wcBuffer);
			lvi.iSubItem = 1;
			lvi.pszText = wcBuffer;
			ListView_SetItem(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), &lvi);

			++p;
		}

		ListView_SetColumnWidth(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), 0, LVSCW_AUTOSIZE_USEHEADER);
		ListView_SetColumnWidth(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), 1, LVSCW_AUTOSIZE_USEHEADER);
		ListView_SetItemState(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), 0, LVIS_SELECTED, LVIS_SELECTED);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_EDTHOTKEY:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				m_bHotKeyChanged = true;
				m_lHotKeyOldIndex = SendMessage(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), LVM_GETNEXTITEM, -1, LVNI_ALL | LVNI_SELECTED);
				break;
			}
			break;
		}

		case IDC_BTNKEYSDEFS:
		{
			if (MessageBox(hwndDlg, L"Are you sure you wish to reset all key bindings to their default values?", L"Reset Key Bindings",
				MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_APPLMODAL) == IDYES)
			{
				ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS));

				delete vKeys;
				vKeys = new vector<HotKeyItem>;
				CreateDefaultHotkeys(vKeys);

				vector<HotKeyItem>::iterator p = vKeys->begin();

				LVITEM lvi;
				lvi.mask = LVIF_TEXT;
				wchar_t wcBuffer[64] = { 0 };
				sHotkey shkTemp;

				while (p != vKeys->end())
				{
					memset(&shkTemp, NULL, sizeof(sHotkey));

					lvi.iItem = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS));
					lvi.iSubItem = 0;
					lvi.pszText = p->wcPrintName;
					ListView_InsertItem(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), &lvi);

					LoadHotkey(*p, &shkTemp, wcBuffer);
					lvi.iSubItem = 1;
					lvi.pszText = wcBuffer;
					ListView_SetItem(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), &lvi);

					++p;
				}

				ListView_SetItemState(GetDlgItem(hwndDlg, IDC_LSTHOTKEYS), 0, LVIS_SELECTED, LVIS_SELECTED);

			}
		}
		}
		retVal = 0;
		break;
	}
	case WM_NOTIFY:
	{
		LPNMLISTVIEW lpnmitem = (LPNMLISTVIEW)lParam;

		if (lpnmitem->hdr.code == LVN_ITEMCHANGED && (lpnmitem->uNewState & LVIS_SELECTED) && !(lpnmitem->uOldState & LVIS_SELECTED))
		{
			if (m_bHotKeyChanged)
			{
				BYTE bMods = 0;
				wchar_t wcBuffer[64] = { 0 };

				if (shkHold.bControl)
					bMods |= MOD_CONTROL;
				if (shkHold.bShift)
					bMods |= MOD_SHIFT;
				if (shkHold.bWin)
					bMods |= MOD_WIN;
				if (shkHold.bAlt)
					bMods |= MOD_ALT;
				if (shkHold.bAppCmd)
					bMods = MOD_APPCOMMAND;

				vKeys->at(m_lHotKeyOldIndex).uiKey = shkHold.uiKey;
				vKeys->at(m_lHotKeyOldIndex).uiMods = bMods;

				m_bHotKeyChanged = false;
			}

			if (-1 != lpnmitem->iItem)
			{
				wchar_t wcBuffer[64] = { 0 };
				sHotkey shkTemp;
				memset(&shkTemp, NULL, sizeof(sHotkey));

				LoadHotkey(vKeys->at(lpnmitem->iItem), &shkTemp, wcBuffer);
				memcpy((void*)&shkHold, (void*)&shkTemp, sizeof(sHotkey));
				memset(&shkBuilding, NULL, sizeof(sHotkey));
				SetDlgItemText(hwndDlg, IDC_EDTHOTKEY, wcBuffer);
				m_bHotKeyChanged = false;
			}
		}

		break;
	}
	case WM_APPLY:
	{
		BYTE bMods = 0;

		if (shkHold.bControl)
			bMods |= MOD_CONTROL;
		if (shkHold.bShift)
			bMods |= MOD_SHIFT;
		if (shkHold.bWin)
			bMods |= MOD_WIN;
		if (shkHold.bAlt)
			bMods |= MOD_ALT;
		if (shkHold.bAppCmd)
			bMods = MOD_APPCOMMAND;

		vKeys->at(m_lHotKeyOldIndex).uiKey = shkHold.uiKey;
		vKeys->at(m_lHotKeyOldIndex).uiMods = bMods;

		return TRUE;
		break;
	}
	case WM_CLOSE:
	{
		RemoveWindowSubclass(GetDlgItem(hwndDlg, IDC_EDTHOTKEY), HotkeyEditSubclassProc, 0);
		retVal = 0;
		break;
	}
	}

	return retVal;
}

LRESULT CALLBACK HotkeyEditSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (message)
	{
	case WM_GETDLGCODE:
	{
		return DLGC_HASSETSEL | DLGC_WANTALLKEYS | DLGC_WANTARROWS | DLGC_WANTCHARS;
	}
	case WM_SETFOCUS:
	{
		RegisterListener(hwndHotkeyDlg);

		return DefSubclassProc(hWnd, message, wParam, lParam);
	}
	case WM_KILLFOCUS:
	{
		if (!g_bKeepHookRegistered)
			UnregisterListener();

		return DefSubclassProc(hWnd, message, wParam, lParam);
	}
	case WM_SYSCHAR:
	{
		return 0;
	}
	case WM_KEYDOWN:
	{
		memset((void *)&shkBuilding, 0, sizeof(sHotkey));
		memset((void *)&shkHold, 0, sizeof(sHotkey));

		SetWindowText(hWnd, L"(None)");
		PostMessage(hwndHotkeyDlg, WM_COMMAND, MAKEWPARAM(IDC_EDTHOTKEY, EN_CHANGE), 0);

		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = SendMessage(GetDlgItem(GetParent(hWnd), IDC_LSTHOTKEYS), LVM_GETNEXTITEM, -1, LVNI_ALL | LVNI_SELECTED);
		lvi.iSubItem = 1;
		lvi.pszText = L"(None)";
		ListView_SetItem(GetDlgItem(GetParent(hWnd), IDC_LSTHOTKEYS), &lvi);

		return 0;
	}
	case WM_CHAR:
	{
		return 0;
	}

	default: //Send all messages not processed to default edit proc
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}
}

int DelItem(HKEY hKey, LPCWSTR lpPath, LPCWSTR lpName)
{
	DWORD dwRet = 0;
	HKEY hkRoot;

	dwRet = RegCreateKeyEx(hKey,
		lpPath,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hkRoot,
		NULL);

	if (dwRet == ERROR_SUCCESS)
	{
		dwRet = RegDeleteValue(hkRoot, lpName);
	}

	RegCloseKey(hkRoot);

	return dwRet;
}

void WindowsStartup(bool bStart)
{
	DWORD dwRet = 0;
	HKEY hkRoot;
	wchar_t wcFilename[MAX_PATH];

	dwRet = RegCreateKeyEx(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run\0",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hkRoot,
		NULL);

	if (bStart)
	{
		if (dwRet == ERROR_SUCCESS)
		{
			dwRet = GetModuleFileName(NULL, (LPWSTR)wcFilename, MAX_PATH);
			PathRemoveFileSpec((LPWSTR)wcFilename);
			PathAppend((LPWSTR)wcFilename, L"\\iTunesCtl.exe");

			if (dwRet != 0)
			{
				dwRet = RegSetValueEx(hkRoot,
					L"iTunesControl",
					0,
					REG_SZ,
					(CONST BYTE*)wcFilename,
					sizeof(wcFilename) / sizeof(wcFilename[0]));
			}
		}
	}
	else
	{
		DelItem(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run\0", L"iTunesControl");
	}

	RegCloseKey(hkRoot);

	return;
}