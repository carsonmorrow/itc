#include "itunescominterface.h"
#include "searchhandlers.h"
#include "memcheck.h"

DWORD WINAPI PlaylistSearchThreadProc(LPVOID)
{
	WNDCLASSEX wc = { 0 };
	static TCHAR szAppName[] = _T("iTC_PlaylistSearch");

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = PlaylistSearchWndProc;
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

	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szAppName, L"Playlist Search", WS_CAPTION | WS_BORDER | WS_POPUP,
		400, 400, 300, 200, NULL, NULL, g_hInstance, NULL);

	m_hWndPlaylistSearch = hWnd;

	// make all the other parts of the window
	m_hWndPlaylistSearchTextBox = CreateWindow(WC_EDIT, NULL, WS_BORDER | WS_CHILD | WS_VISIBLE, 0, 0, 295, 20, hWnd, NULL, g_hInstance, NULL);
	m_hWndPlaylistSearchListBox = CreateWindow(WC_LISTBOX, NULL, LBS_STANDARD | WS_CHILD | WS_VISIBLE, 0, 20, 295, 200 - 5 - 20, hWnd, NULL, g_hInstance, NULL);
	SendMessage(m_hWndPlaylistSearchTextBox, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	SendMessage(m_hWndPlaylistSearchListBox, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	SetWindowSubclass(m_hWndPlaylistSearchTextBox, PlaylistSearchTextBoxProc, 0, NULL);

	ShowWindow(hWnd, SW_HIDE);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK PlaylistSearchWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SHOWWINDOW:
		if (FALSE == wParam) // being hidden
			SendMessage(m_hWndPlaylistSearchListBox, LB_RESETCONTENT, 0, 0);
		return 0;
		break;
	case WM_TIMER:
	{
		KillTimer(hwnd, 10);
		SendMessage(m_hWndPlaylistSearchListBox, LB_RESETCONTENT, 0, 0);

		int nTextLength = GetWindowTextLength(m_hWndPlaylistSearchTextBox) + 1;
		if (1 == nTextLength)
		{
			std::vector<std::wstring>::iterator p = vPlaylists.begin();
			while (p != vPlaylists.end())
			{
				SendMessage(m_hWndPlaylistSearchListBox, LB_ADDSTRING, 0, (LPARAM)p->c_str());
				p++;
			}
			return 0;
		}

		wchar_t *wcText = (wchar_t*)malloc(sizeof(wchar_t) * nTextLength);
		wchar_t *wcPlaylist = NULL;

		GetWindowText(m_hWndPlaylistSearchTextBox, wcText, nTextLength);

		std::vector<std::wstring>::iterator p = vPlaylists.begin();
		while (p != vPlaylists.end())
		{
			wcPlaylist = _wcsdup(p->c_str());
			if (NULL != wcsstr(_wcslwr(wcPlaylist), _wcslwr(wcText)))
			{
				SendMessage(m_hWndPlaylistSearchListBox, LB_ADDSTRING, 0, (LPARAM)p->c_str());
			}
			free(wcPlaylist);
			p++;
		}

		SendMessage(m_hWndPlaylistSearchListBox, LB_SETCURSEL, 0, 0);

		free(wcText);

		return 0;
		break;
	}
	case WM_COMMAND:
	{
		if ((HWND)lParam == m_hWndPlaylistSearchTextBox && HIWORD(wParam) == EN_CHANGE)
		{
			KillTimer(hwnd, 10);
			SetTimer(hwnd, 10, 200, NULL);
		}
	}
	break;
	case WM_QUITTHREAD:
	{
		DestroyWindow(hwnd);
		return 0;
	}
	case WM_DESTROY:
		if (lpwcQuickPlaylist == NULL)
		{
			free(lpwcQuickPlaylist);
			lpwcQuickPlaylist = NULL;
		}
		RemoveWindowSubclass(m_hWndPlaylistSearchTextBox, PlaylistSearchTextBoxProc, 0);
		PostQuitMessage(0);
		return 0;
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK PlaylistSearchTextBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		if (wParam == VK_DOWN)
		{
			long lItem = SendMessage(m_hWndPlaylistSearchListBox, LB_GETCURSEL, 0, 0) + 1;
			SendMessage(m_hWndPlaylistSearchListBox, LB_SETCURSEL, lItem, 0);

			return 0;
		}
		else if (wParam == VK_UP)
		{
			long lItem = SendMessage(m_hWndPlaylistSearchListBox, LB_GETCURSEL, 0, 0) - 1;
			if (0 > lItem) lItem = 0;
			SendMessage(m_hWndPlaylistSearchListBox, LB_SETCURSEL, lItem, 0);

			return 0;
		}
	}
	case WM_CHAR:
	{
		if (wParam == VK_ESCAPE)
		{
			ShowWindow(m_hWndPlaylistSearch, SW_HIDE);
			SendMessage(m_hWndPlaylistSearchListBox, LB_RESETCONTENT, 0, 0);
			SetWindowText(m_hWndPlaylistSearchListBox, L"");
			return 0;
		}
		else if (wParam == VK_RETURN)
		{
			long lItem = SendMessage(m_hWndPlaylistSearchListBox, LB_GETCURSEL, 0, 0);
			if (-1 == lItem)
				return 0;
			int nTextLength = SendMessage(m_hWndPlaylistSearchListBox, LB_GETTEXTLEN, lItem, 0) + 1;
			if (lpwcQuickPlaylist == NULL)
			{
				free(lpwcQuickPlaylist);
				lpwcQuickPlaylist = NULL;
			}
			lpwcQuickPlaylist = (wchar_t*)malloc(sizeof(wchar_t) * nTextLength);

			SendMessage(m_hWndPlaylistSearchListBox, LB_GETTEXT, lItem, (LPARAM)lpwcQuickPlaylist);
			SendMessage(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), WM_QUICKPLAYLIST, NULL, NULL);

			ShowWindow(m_hWndPlaylistSearch, SW_HIDE);
			SetWindowText(m_hWndPlaylistSearchTextBox, L"");
			return 0;
		}
	}
	break;
	default:
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	return DefSubclassProc(hWnd, message, wParam, lParam);
}

DWORD WINAPI SongSearchThreadProc(LPVOID)
{
	WNDCLASSEX wc = { 0 };
	static TCHAR szAppName[] = _T("iTC_SongSearch");

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = SongSearchWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szAppName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szAppName, L"Song Search", WS_CAPTION | WS_POPUPWINDOW,
		400, 400, 300, 200, NULL, NULL, g_hInstance, NULL);

	m_hWndSongSearch = hWnd;

	// make all the other parts of the window
	m_hWndSongSearchTextBox = CreateWindow(WC_EDIT, NULL, WS_BORDER | WS_CHILD | WS_VISIBLE, 0, 0, 295, 20, hWnd, NULL, g_hInstance, NULL);
	m_hWndSongSearchListBox = CreateWindow(WC_LISTBOX, NULL, LBS_STANDARD | WS_CHILD | WS_VISIBLE, 0, 20, 295, 200 - 5 - 20, hWnd, NULL, g_hInstance, NULL);
	SendMessage(m_hWndSongSearchTextBox, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	SendMessage(m_hWndSongSearchListBox, WM_SETFONT, (WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	SetWindowSubclass(m_hWndSongSearchTextBox, SongSearchTextBoxProc, 0, NULL);
	SetWindowSubclass(m_hWndSongSearchListBox, SongSearchListBoxProc, 0, NULL);

	ShowWindow(hWnd, SW_HIDE);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK SongSearchWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SHOWWINDOW:
		if (FALSE == wParam) // being hidden
			SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
		else
		{
			SetFocus(m_hWndSongSearchTextBox);
			SetWindowText(hwnd, L"Song Search");
		}

		return 0;
		break;
	case WM_TIMER:
	{
		KillTimer(hwnd, 10);
		SetEvent(g_hSS_Stop);
		if (WAIT_TIMEOUT == WaitForSingleObject(g_hSS_Stopped, 0))
		{
			SetTimer(hwnd, 10, 1, NULL);
			return 0;
		}

		EnterCriticalSection(&g_csSongSearch);
		ResetEvent(g_hSS_Stop);

		SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
		SetWindowText(m_hWndSongSearch, L"Song Search");

		int nTextLength = GetWindowTextLength(m_hWndSongSearchTextBox) + 1;
		if (1 == nTextLength)
		{
			SetWindowText(hwnd, L"Song Search");
			LeaveCriticalSection(&g_csSongSearch);
			return 0;
		}

		if (NULL != lpwcSongSearch)
			free(lpwcSongSearch);
		lpwcSongSearch = (wchar_t*)malloc(sizeof(wchar_t) * nTextLength);

		GetWindowText(m_hWndSongSearchTextBox, lpwcSongSearch, nTextLength);

		ResetEvent(g_hSS_Stopped);
		PostMessage(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), WM_SONGSEARCH, NULL, NULL);
		LeaveCriticalSection(&g_csSongSearch);

		return 0;
		break;
	}
	case WM_COMMAND:
	{
		if ((HWND)lParam == m_hWndSongSearchTextBox && HIWORD(wParam) == EN_CHANGE)
		{
			KillTimer(hwnd, 10);
			SetTimer(hwnd, 10, 100, NULL);
		}
	}
	break;
	case WM_QUITTHREAD:
	{
		DestroyWindow(hwnd);
		return 0;
	}
	case WM_CLOSE:
	{
		ShowWindow(m_hWndSongSearch, SW_HIDE);
		SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
		SetWindowText(m_hWndSongSearchTextBox, L"");

		return 0;
	}
	case WM_DESTROY:
		SetEvent(g_hSS_Stop);
		// Can't really wait here, we'll be blocking
		EnterCriticalSection(&g_csSongSearch);
		if (NULL != lpwcSongSearch)
		{
			free(lpwcSongSearch);
			lpwcSongSearch = NULL;
		}
		LeaveCriticalSection(&g_csSongSearch);

		RemoveWindowSubclass(m_hWndSongSearchTextBox, SongSearchTextBoxProc, 0);
		RemoveWindowSubclass(m_hWndSongSearchListBox, SongSearchListBoxProc, 0);
		PostQuitMessage(0);
		return 0;
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK SongSearchTextBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		if (wParam == VK_DOWN)
		{
			long lItem = SendMessage(m_hWndSongSearchListBox, LB_GETCURSEL, 0, 0) + 1;
			SendMessage(m_hWndSongSearchListBox, LB_SETCURSEL, lItem, 0);

			return 0;
		}
		else if (wParam == VK_UP)
		{
			long lItem = SendMessage(m_hWndSongSearchListBox, LB_GETCURSEL, 0, 0) - 1;
			if (0 > lItem) lItem = 0;
			SendMessage(m_hWndSongSearchListBox, LB_SETCURSEL, lItem, 0);

			return 0;
		}
		else if (wParam == VK_PRIOR)
		{
			PostMessage(m_hWndSongSearchListBox, WM_KEYDOWN, wParam, lParam);

			return 0;
		}
		else if (wParam == VK_NEXT)
		{
			PostMessage(m_hWndSongSearchListBox, WM_KEYDOWN, wParam, lParam);

			return 0;
		}
	}
	case WM_CHAR:
	{
		if (wParam == VK_ESCAPE)
		{
			ShowWindow(m_hWndSongSearch, SW_HIDE);
			SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
			SetWindowText(m_hWndSongSearchTextBox, L"");

			return 0;
		}
		else if (wParam == VK_RETURN)
		{
			long lTrackID = 0;
			long lItem = SendMessage(m_hWndSongSearchListBox, LB_GETCURSEL, 0, 0);
			if (-1 == lItem)
				return 0;

			lTrackID = SendMessage(m_hWndSongSearchListBox, LB_GETITEMDATA, lItem, 0);
			SendMessage(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), WM_PLAYSSRESULT, LOWORD(lTrackID), HIWORD(lTrackID));

			ShowWindow(m_hWndSongSearch, SW_HIDE);
			SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
			SetWindowText(m_hWndSongSearchTextBox, L"");

			return 0;
		}
	}
	break;
	default:
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	return DefSubclassProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK SongSearchListBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
	switch (message)
	{
	case WM_CHAR:
	{
		if (wParam == VK_ESCAPE)
		{
			ShowWindow(m_hWndSongSearch, SW_HIDE);
			SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
			SetWindowText(m_hWndSongSearchTextBox, L"");

			return 0;
		}
		else if (wParam == VK_RETURN)
		{
			long lTrackID = 0;
			long lItem = SendMessage(m_hWndSongSearchListBox, LB_GETCURSEL, 0, 0);
			if (-1 == lItem)
				return 0;

			lTrackID = SendMessage(m_hWndSongSearchListBox, LB_GETITEMDATA, lItem, 0);
			SendMessage(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), WM_PLAYSSRESULT, LOWORD(lTrackID), HIWORD(lTrackID));

			ShowWindow(m_hWndSongSearch, SW_HIDE);
			SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
			SetWindowText(m_hWndSongSearchTextBox, L"");

			return 0;
		}
	}
	break;
	case WM_LBUTTONDBLCLK:
	{
		long lTrackID = 0;
		long lItem = SendMessage(m_hWndSongSearchListBox, LB_GETCURSEL, 0, 0);
		if (-1 == lItem)
			return 0;

		lTrackID = SendMessage(m_hWndSongSearchListBox, LB_GETITEMDATA, lItem, 0);
		SendMessage(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), WM_PLAYSSRESULT, LOWORD(lTrackID), HIWORD(lTrackID));

		ShowWindow(m_hWndSongSearch, SW_HIDE);
		SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, 0, 0);
		SetWindowText(m_hWndSongSearchTextBox, L"");

		return 0;
	}
	break;
	default:
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	return DefSubclassProc(hWnd, message, wParam, lParam);
}