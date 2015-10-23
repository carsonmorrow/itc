#include "itunesctl.h"
#include "memcheck.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	ENABLE_LEAK_CHECK

		WNDCLASSEX wc = { 0 };
	static TCHAR szAppName[] = _T("iTunesControl");
	HANDLE hMutex = NULL;
	wchar_t wcLocalizedMsg[50] = { 0 };

	//check for previous instance first
	hMutex = CreateMutex(NULL, TRUE, L"iTunesControlMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWndITCTemp = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");
		if (hWndITCTemp != NULL)
			PostMessage(hWndITCTemp, WM_CONNECT, NULL, NULL);
		ReleaseMutex(hMutex);
		return 0;
	}

	vector<HotKeyItem> *vKeys = new vector<HotKeyItem>;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szAppName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindowEx(0, szAppName, _T("iTunesControl"), 0, CW_USEDEFAULT,
		CW_USEDEFAULT, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

	g_hInstance = hInstance;

	// basic startup
	LoadSettings(vKeys, &m_ssSettings);

	ApplyHotKeys(vKeys);
	SetMatchMode(true);
	RegisterListener(hWnd);

	if (m_ssSettings.bShowInTray)
	{
		st.Create(hWnd, L"Connected", LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGO)), 0, WM_ICONTRAY);
	}

	g_hSS_Stop = CreateEvent(NULL, TRUE, TRUE, NULL);
	g_hSS_Stopped = CreateEvent(NULL, TRUE, TRUE, NULL);
	InitializeCriticalSection(&g_csSongSearch);

	m_ITCom = new CITunesCOM();

	// Start up GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);

	_ginterface = new GraphicsInterface();
	inputWindow = new InputWindow();

	InitInputEngine();
	hPlaylistSearchThread = CreateThread(NULL, 0, PlaylistSearchThreadProc, NULL, 0, NULL);
	hSongSearchThread = CreateThread(NULL, 0, SongSearchThreadProc, NULL, 0, NULL);

	m_ITCom->Connect(m_ssSettings.bStartiTunesAtStart);
	SetACIgnore(m_ssSettings.bDisableACOnFocus, m_ITCom->GetiTunesHWND());

	SetState(m_ITCom->IsConnected());

	m_bTrackChangeAllowed = true;

	ProcessCommandLine(hWnd);
	//End basic startup

	// Track time update
	SetTimer(hWnd, 15, DISPLAY_UPDATE_PERIOD, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// No more hotkeys
	ClearCombos();
	UnregisterListener();

	// Ask all threads to quit, please
	PostMessage(m_hWndPlaylistSearch, WM_QUITTHREAD, 0, 0);
	PostMessage(m_hWndSongSearch, WM_QUITTHREAD, 0, 0);

	HANDLE hThreads[2] = { hPlaylistSearchThread, hSongSearchThread };
	WaitForMultipleObjects(2, hThreads, TRUE, 10000);

	m_ITCom->SetWindowCaption(true);

	m_ITCom->Disconnect();

	delete m_ITCom;
	CloseHandle(g_hSS_Stop);
	CloseHandle(g_hSS_Stopped);
	DeleteCriticalSection(&g_csSongSearch);

	delete _ginterface;
	_ginterface = NULL;
	ShutdownInputEngine();

	// Shut down GDI+
	GdiplusShutdown(_gdiplusToken);

	delete vKeys;

	ReleaseMutex(hMutex);

	st.Destroy();

	return (int)msg.wParam;
}

void ProcessCommandLine(HWND hWnd)
{
	LPWSTR *szArglist = NULL;
	int nArgs = 0;
	int nSleepTime = 0;
	bool bSleepTime = false;
	int nSleepType = 0;
	bool bSleepType = false;

	szArglist = CommandLineToArgvW(GetCommandLine(), &nArgs);

	if (nArgs > 1)
	{
		for (int i = 1; i < nArgs; i++)
		{
			if (0 == wcsicmp(szArglist[i], L"-sleeptime"))
			{
				// i+1 is time
				nSleepTime = _wtoi(szArglist[i + 1]);
				bSleepTime = true;
			}
			else if (0 == wcsicmp(szArglist[i], L"-sleeptype"))
			{
				// i+1 is type
				nSleepType = _wtoi(szArglist[i + 1]);
				bSleepType = true;
			}
			else if (0 == wcsicmp(szArglist[i], L"-itunesstart"))
			{
				// we need to connect as soon as we can
				SetTimer(hWnd, 10, 5000, NULL);
			}
		}

		if (bSleepType && bSleepTime)
		{
			m_ssSettings.nSleepType = nSleepType;
			m_ITCom->SetSleepTimer(nSleepTime);
		}
	}

	LocalFree(szArglist);

	return;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_RELOADSETTINGS == message)
	{
		//Wait 500 ms for another message before actually doing it
		KillTimer(hWnd, 1423);
		SetTimer(hWnd, 1423, 500, NULL);

		return 0;
	}
	else if (WM_HOTKEYSACTION == message)
	{
		if (wParam == 1)
		{
			UnApplyHotKeys();
			UnregisterListener();
			SetACIgnore(false, NULL);
		}
		else
		{
			vector<HotKeyItem> *vKeys = new vector<HotKeyItem>;
			LoadSettings(vKeys, NULL);
			ApplyHotKeys(vKeys);
			SetMatchMode(true);
			RegisterListener(hWnd);
			SetACIgnore(m_ssSettings.bDisableACOnFocus, m_ITCom->GetiTunesHWND());
			delete vKeys;
		}
		return 0;
	}
	else if (HWM_HOOKMSG == message)
	{
		return ProcessHotkey(hWnd, wParam);
	}

	switch (message)
	{
	case WM_HOTKEY:
	{
		return ProcessHotkey(hWnd, wParam);
		break;
	}

	case WM_EXTHOTKEY:
	{
		return ProcessHotkey(hWnd, wParam);
		break;
	}

	case WM_SONGSEARCH:
	{
		SetTimer(hWnd, 1065, 1, NULL);
		return 0;
		break;
	}
	case WM_PLAYSSRESULT:
	{
		_lTrackID = wParam;
		_lTrackDBID = lParam;
		SetTimer(hWnd, 1066, 1, NULL);
		_sShift = GetAsyncKeyState(VK_SHIFT);
		_sCtrl = GetAsyncKeyState(VK_CONTROL);
		return 0;
		break;
	}

	case WM_QUICKPLAYLIST:
	{
		SetTimer(hWnd, 1599, 1, NULL);

		return 0;
		break;
	}

	case WM_STARTWITHITUNES:
	{
		SetTimer(hWnd, 10, 5000, NULL);

		return 0;
		break;
	}

	case WM_ICONTRAY:
	{
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
		{
			POINT ptCur;
			MENUITEMINFO miiItem;

			GetCursorPos(&ptCur);
			HMENU hPopUp = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUP));
			HMENU hSubMenu = GetSubMenu(hPopUp, 0);

			miiItem.cbSize = sizeof(MENUITEMINFO);
			miiItem.fMask = MIIM_STRING;
			miiItem.fType = MFT_STRING;
			if (m_ITCom->IsConnected())
				miiItem.dwTypeData = L"&Disconnect";
			else
				miiItem.dwTypeData = L"&Connect";

			SetMenuItemInfo(hSubMenu, ID_POPUP_CONNECT, FALSE, &miiItem);

			SetForegroundWindow(hWnd);
			TrackPopupMenuEx(hSubMenu, 0, ptCur.x, ptCur.y, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
			DestroyMenu(hSubMenu);
			DestroyMenu(hPopUp);
			break;
		}

		case WM_LBUTTONDBLCLK:
		{
			if (!m_ITCom->IsConnected())
			{
				//stabilize first
				m_ITCom->Stabilize();

				//then start it up
				m_ITCom->Connect(true);

				SetState(m_ITCom->IsConnected());
			}
			else
			{
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				wchar_t wcFilename[MAX_PATH] = { 0 };

				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&pi, sizeof(pi));

				DWORD dwRet = GetModuleFileName(NULL, (LPWSTR)wcFilename, sizeof(wcFilename) / sizeof(wcFilename[0]));
				PathRemoveFileSpec((LPWSTR)wcFilename);
				PathAppend((LPWSTR)wcFilename, L"\\config.exe");

				dwRet = CreateProcess(wcFilename, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

			}

			break;
		}
		}

		return 0;
		break;
	}

	case WM_DISPLAYCHANGE:
	{
		CMonitors cm;
		if (m_ssSettings.nOSDMonitor > cm.GetCount()) //if we've lost our monitor
		{
			m_ssSettings.nOSDMonitor = 1;
		}

		return 0;
		break;
	}

	case WM_TIMER:
	{
		switch (wParam)
		{
		case 2: //track change delay
		{
			m_bTrackChangeAllowed = true;
			KillTimer(hWnd, 2);
			return 0;
			break;
		}

		case 10: // start with itunes timer
		{
			if (m_ITCom->IsConnected())
			{
				KillTimer(hWnd, 10);
				SetState(m_ITCom->IsConnected());
			}
			else
				m_ITCom->Stabilize();

			return 0;
			break;
		}
		case 15: // Track time update
		{
			if (m_ITCom->IsPlaying())
			{
				if (m_ssSettings.bNoDisplayShortTracks && m_ITCom->GetTrackLength() < m_ssSettings.nShortTrackLength)
					return 0;

				if (NULL != wcsstr(m_ssSettings.wcOSDFormat, L"%track_position%"))
					m_ITCom->ShowCurrentInfo(m_ssSettings.nPopupShowTime, true);

				if (m_ssSettings.bShowDisplayAtEnd && !m_ssSettings.bOSDAlwaysUp &&
					(m_ITCom->GetTimeLeftInTrack() - m_ssSettings.nShowEndOffset / 1000) == m_ssSettings.nDisplayEndTime / 1000)
				{
					m_ITCom->ShowCurrentInfo(m_ssSettings.nDisplayEndTime);
				}

				if (m_ssSettings.bShowDisplayAtStart && !m_ssSettings.bOSDAlwaysUp &&
					m_ssSettings.nShowBeginOffset != 0 && m_ITCom->GetTimeInTrack() == m_ssSettings.nShowBeginOffset / 1000)
				{
					m_ITCom->ShowCurrentInfo(m_ssSettings.nDisplayStartTime);
				}
			}
			return 0;
			break;
		}
		case 1599: //quick playlist timer
		{
			KillTimer(hWnd, 1599);
			wstring wstrTemp = lpwcQuickPlaylist;
			free(lpwcQuickPlaylist);
			lpwcQuickPlaylist = NULL;
			m_ITCom->QuickPlaylist(wstrTemp);
			return 0;
			break;
		}
		case 1065: // song search
		{
			KillTimer(hWnd, 1065);
			// enter critical section
			EnterCriticalSection(&g_csSongSearch);
			wstring wstrTemp = lpwcSongSearch;
			free(lpwcSongSearch);
			lpwcSongSearch = NULL;
			// leave critical section
			LeaveCriticalSection(&g_csSongSearch);
			m_ITCom->SongSearch(wstrTemp);
			SetEvent(g_hSS_Stopped);
			return 0;
			break;
		}
		case 1066: // play song search
		{
			KillTimer(hWnd, 1066);

			if (_sShift < 0)
				SongSearchAction(m_ssSettings.nSSShiftEnterAction);
			else if (_sCtrl < 0)
				SongSearchAction(m_ssSettings.nSSCtrlEnterAction);
			else
				SongSearchAction(m_ssSettings.nSSEnterAction);

			return 0;
			break;
		}
		case 1320: //sleep timer
		{
			switch (m_ssSettings.nSleepType)
			{
			case 0:
			{
				_ginterface->DisplayStatus(L"Sleep Timer pausing iTunes");
				m_ITCom->Pause();
				break;
			}
			case 1:
			{
				_ginterface->DisplayStatus(L"Sleep Timer shutting down iTunes");
				m_ITCom->Quit();
				SetState(m_ITCom->IsConnected());
				break;
			}
			case 2:
			{
				_ginterface->DisplayStatus(L"Sleep Timer shutting down Windows");
				m_ITCom->Quit();

				PowerControl(0);
				break;
			}
			case 3:
			{
				_ginterface->DisplayStatus(L"Sleep Timer suspending Windows");
				m_ITCom->Pause();

				PowerControl(1);
				break;
			}
			case 4:
			{
				_ginterface->DisplayStatus(L"Sleep Timer hibernating Windows");
				m_ITCom->Pause();

				PowerControl(2);
				break;
			}
			case 5:
			{
				_ginterface->DisplayStatus(L"Sleep Timer pausing iTunes and locking Windows");
				m_ITCom->Pause();
				LockWorkStation();
				break;
			}
			case 6:
			{
				_ginterface->DisplayStatus(L"Sleep Timer locking Windows");
				LockWorkStation();
				break;
			}
			}

			KillTimer(hWnd, 1320);
			m_ITCom->m_bSleepOn = false;

			return 0;
			break;
		}
		case 1423: // Reload settings
		{
			KillTimer(hWnd, 1423);

			LoadSettings(NULL, &m_ssSettings);
			// Track time update
			SetTimer(hWnd, 15, DISPLAY_UPDATE_PERIOD, NULL);

			if (m_ssSettings.bShowInTray)
			{
				if (!st.IsActive()) //only create if not already created.
				{
					st.Create(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), L"Connected", LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LOGO)), 0, WM_ICONTRAY);
					SetState(m_ITCom->IsConnected());
				}
			}
			else
			{
				st.Destroy();
			}

			m_ITCom->ReloadArtwork();
			m_ITCom->ShowCurrentInfo();
			m_ITCom->SetWindowCaption();

			return 0;
			break;
		}
		}

		return 0;
		break;
	}

	case WM_CONNECT:
	{
		if (!m_ITCom->IsConnected())
			m_ITCom->Connect(true);

		SetState(m_ITCom->IsConnected());

		return 0;
		break;
	}
	case WM_SETSTATE:
	{
		Sleep(500); //we just need a delay

		if (m_ssSettings.bQuitWithiTunes)
		{
			PostQuitMessage(0);
		}
		else
		{
			SetState(m_ITCom->IsConnected());
		}

		return 0;
		break;
	}
	case WM_COMMAND:
	{
		// Just to be extra safe, check that we actually are getting menu messages
		if (HIWORD(wParam) != 0 || lParam != 0)
			return 0;

		switch (LOWORD(wParam))
		{
		case ID_POPUP_SETTINGS:
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			wchar_t wcFilename[MAX_PATH] = { 0 };

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));

			DWORD dwRet = GetModuleFileName(NULL, (LPWSTR)wcFilename, sizeof(wcFilename) / sizeof(wcFilename[0]));
			PathRemoveFileSpec((LPWSTR)wcFilename);
			PathAppend((LPWSTR)wcFilename, L"\\config.exe");

			dwRet = CreateProcess(wcFilename, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			break;
		}

		case ID_POPUP_EXIT:
		{
			//settings window still open
			if (!m_ITCom->IsConnected())
			{
				PostQuitMessage(0);
				break;
			}

			try
			{
				if (m_ssSettings.bQuitiTunesOnExit)
				{
					if (m_ssSettings.bPromptQuitiTunes)
					{
						int nResult = MessageBox(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), L"Quit iTunes?", L"iTunesControl",
							MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);

						if (nResult == IDYES)
						{
							m_ITCom->Quit();
						}
						else if (nResult == IDCANCEL)
						{
							break;
						}
					}
					else
					{
						m_ITCom->Quit();
					}
				}
				PostQuitMessage(0);
			}
			catch (...)
			{
				_RPTFW0(_CRT_WARN, L"Caught exception while trying to quit.");
				PostQuitMessage(0);
			}

			break;
		}

		case ID_POPUP_ABOUT:
		{
			HWND hWndAbout = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
			ShowWindow(hWndAbout, SW_SHOW);

			break;
		}

		case ID_POPUP_CONNECT:
		{
			if (m_ITCom->IsConnected())
			{
				_ginterface->DisplayHide();
				m_ITCom->Disconnect();
			}
			else
				m_ITCom->Connect(true);

			SetState(m_ITCom->IsConnected());

			break;
		}
		}

		return 0;
		break;
	}

	case WM_DESTROY:
	{
		UnregisterListener();
		UnApplyHotKeys();

		return 0;
	}

	case WM_QUERYENDSESSION:
	{
		return TRUE;
	}

	case WM_ENDSESSION:
	{
		//we have to close; just call ->quit()
		if (m_ITCom->IsConnected())
		{
			m_ITCom->Quit();
		}

		PostQuitMessage(0);

		return 0; //we're closing
	}

	case WM_CLOSE:
	{
		//if we didn't respond to wm_endsession, we get this
		if (m_ITCom->IsConnected())
		{
			m_ITCom->Quit();
		}

		PostQuitMessage(0);

		return 0; //we're closing
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

INT_PTR CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	int retVal = 0;

	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		wchar_t wcsStamps[2500];

		swprintf_s(wcsStamps, L"iTunesControl %s (Build %s)\r\n\r\nCopyright (c) 2004-2015 Carson Morrow\r\nAll rights reserved.\r\n\r\n" \
			L"Redistribution and use in binary form, without modification, are permitted provided that the following conditions are met:\r\n\r\n" \
			L"Only unmodified installers can be redistributed; redistribution of iTunesControl binaries in any other form is not permitted.\r\n" \
			L"Neither the name of the author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.\r\n\r\n" \
			L"Except as and only to the extent permitted in this License and by applicable law, you may not copy, decompile, reverse engineer, disassemble, modify, or create derivative works of the software or any part thereof.\r\n\r\n" \
			L"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY" \
			L" AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES" \
			L" (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN" \
			L" CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\r\n\r\n" \
			L"Note that a separate less restrictive license applies to open source parts, downloadable separately.\r\n\r\n" \
			L"pugixml (http://pugixml.org)\r\nCopyright (C) 2006-2015, by Arseny Kapoulkine\r\n\r\n" \
			L"TEventHandler (http://www.codeproject.com/KB/COM/TEventHandler.aspx)\r\nCopyright (c) 2004 Lim Bio Liong. All rights reserved.\r\n" \
			L"Distributed under the terms of COPL (http://www.codeproject.com/info/cpol10.aspx).", PRETTYVERSION, BUILDVERSION);
		SetDlgItemText(hwndDlg, IDC_EDTABOUT, wcsStamps);

		retVal = TRUE;
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_AB_OK:
		{
			DestroyWindow(hwndDlg);
			break;
		}
		}
		break;
	}
	}

	return retVal;
}

void SongSearchAction(int nType)
{
	switch (nType)
	{
	case 0: // Just start playing result
		m_ITCom->PlaySSResult(_lTrackID, _lTrackDBID);
		break;
	case 1: // Add to current playlist and start playing
		m_ITCom->PlayInCurrentPlaylist(_lTrackID, _lTrackDBID);
		break;
	default: // Enqueue in iTunes DJ as next track
		m_ITCom->PlayIniTunesDJ(_lTrackID, _lTrackDBID);
		break;
	}

	return;
}