#pragma once

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commdlg.h>
#include <shellapi.h>
#include "gdiplus_mem.h"

#include <string>
using namespace std;

#include "../itc/settings.h"
#include "../common/multimon.h"
#include "resource.h"

INT_PTR CALLBACK MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DisplaySheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ArtSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MiscSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StartShutSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK HotkeysSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LayoutSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ColorsFontsSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PositionSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FeaturesSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvancedSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SongSearchSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RatingsSheetDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK PicPositionSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
LRESULT CALLBACK HotkeyEditSubclassProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );

HTREEITEM htiDisplay = NULL;
HTREEITEM htiLayout = NULL;
HTREEITEM htiHotkeys = NULL;
HTREEITEM htiMisc = NULL;
HTREEITEM htiStartShut = NULL;
HTREEITEM htiColorsFonts = NULL;
HTREEITEM htiAlbumArt = NULL;
HTREEITEM htiPosition = NULL;
HTREEITEM htiFeatures = NULL;
HTREEITEM htiAdvanced = NULL;
HTREEITEM htiSongSearch = NULL;
HTREEITEM htiRatings = NULL;

HWND _hwndCurrentPage = NULL;

HWND _hwndDisplay = NULL;
HWND _hwndLayout = NULL;
HWND _hwndHotkeys = NULL;
HWND _hwndMisc = NULL;
HWND _hwndAlbumArt = NULL;
HWND _hwndStartShut = NULL;
HWND _hwndColorsFonts = NULL;
HWND _hwndPosition = NULL;
HWND _hwndFeatures = NULL;
HWND _hwndAdvanced = NULL;
HWND _hwndSongSearch = NULL;
HWND _hwndRatings = NULL;

HINSTANCE g_hInstance = NULL;

inline void SwitchPages( HWND hwndNew )
{
	if ( _hwndCurrentPage != hwndNew )
	{
		ShowWindow( _hwndCurrentPage, SW_HIDE );
		ShowWindow( hwndNew, SW_NORMAL );
		_hwndCurrentPage = hwndNew;
	}
	return;
}

void WindowsStartup(bool bStart);
int DelItem(HKEY hKey, LPCWSTR lpPath, LPCWSTR lpName);

HBITMAP LoadAnImage(wchar_t* wcFilename);

vector<HotKeyItem> *vKeys;
vector<HotKeyItem> *vOriginal;
SettingsStruct m_ssSettings;
SettingsStruct _ssOriginal;
HWND hwndHotkeyDlg = NULL;
bool g_bKeepHookRegistered = false;

HBRUSH g_hbrBkgnd = NULL;
HBRUSH g_hbrFont = NULL;
HBRUSH g_hbrBorder = NULL;
HBRUSH g_hbrArtBorder = NULL;
HBRUSH g_hbrDropShadow = NULL;
HFONT g_hFont = NULL;

HBRUSH g_hbrShapeFill = NULL;
HBRUSH g_hbrShapeOutline = NULL;

ULONG_PTR g_gdiplusToken;
HBITMAP g_hbmFullStar = NULL;
HBITMAP g_hbmHalfStar = NULL;
HBITMAP g_hbmEmptyStar = NULL;

#define HOTKEY_KEY_LEN 24
struct sHotkey
{
	wchar_t wcKey[HOTKEY_KEY_LEN];
	UINT uiKey;
	bool bWin;
	bool bShift;
	bool bControl;
	bool bAlt;
	bool bSingle;
	bool bAppCmd;
};

wchar_t wcConstants[15][20] = {
	{L"%album%\0"},
	{L"%artist%\0"},
	{L"%bit_rate%\0"},
	{L"%comment%\0"},
	{L"%composer%\0"},
	{L"%genre%\0"},
	{L"%rating%\0"},
	{L"%tags%\0"},
	{L"%total_tracks%\0"},
	{L"%track_length%\0"},
	{L"%tracknumber%\0"},
	{L"%track_position%\0"},
	{L"%track%\0"},
	{L"%year%\0"},
	{L"%normalize%\0"}
};

bool ValidateHotkey( WPARAM wParam, LPARAM lParam, wchar_t *wcBuffer, sHotkey *shkHotkey );
bool ValidateAppCmd( WPARAM wParam, wchar_t *wcBuffer, sHotkey *shkHotkey );
bool LoadHotkey( HotKeyItem hkiHotkey, sHotkey *shkHotkey, wchar_t *wcBuffer );
bool MakeHotkeyText( sHotkey shkHotkey, wchar_t *wcBuffer );

sHotkey shkBuilding = {0};
sHotkey shkHold = {0};
bool bHold = false;

wchar_t wcExtraKeys[18][24] = {
	{L"Browser Back\0"},
	{L"Browser Forward\0"},
	{L"Browser Refresh\0"},
	{L"Browser Stop\0"},
	{L"Browser Search\0"},
	{L"Browser Favorites\0"},
	{L"Browser Home\0"},
	{L"Volume Mute\0"},
	{L"Volume Down\0"},
	{L"Volume Up\0"},
	{L"Next Track\0"},
	{L"Previous Track\0"},
	{L"Stop\0"},
	{L"Play / Pause\0"},
	{L"Launch Mail\0"},
	{L"Select Media\0"},
	{L"Launch Application 1\0"},
	{L"Launch Application 2\0"}
};

bool m_bHotKeyChanged;
long m_lHotKeyOldIndex;

static const UINT WM_RELOADSETTINGS = RegisterWindowMessage( L"RELOAD_SETTINGS-{B77A1ADA-6421-45c8-A1E1-9B99D85E1486}" );
static const UINT WM_HOTKEYSACTION = RegisterWindowMessage( L"HOTKEYS_ACTION-{F44C184E-A088-4b7f-A24E-90CA8C68FE1B}" );
static const UINT HWM_HOOKMSG = RegisterWindowMessage( HWM_HOOKMSG_STR );
const int WM_APPLY = WM_APP + 8;

#ifdef _DEBUG
#define _DBGLOG(msg) (void)(OutputDebugString(msg))
#else
#define _DBGLOG(msg)
#endif