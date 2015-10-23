#include "globaldata.h"
#include "core.h"
#include "settings.h"

HWND m_hWndInput = NULL;
HWND m_hWndPlaylistSearch = NULL;
HWND m_hWndSongSearch = NULL;
SettingsStruct m_ssSettings;
bool m_bTrackChangeAllowed;

CITunesCOM *m_ITCom;
InputWindow *inputWindow = NULL;
vector<wstring> vPlaylists;
GraphicsInterface *_ginterface = NULL;

HINSTANCE g_hInstance;
SysTray st;

HANDLE hPlaylistSearchThread = NULL;
HANDLE hSongSearchThread = NULL;

HWND m_hWndSongSearchListBox = NULL;
HWND m_hWndSongSearchTextBox = NULL;
HWND m_hWndPlaylistSearchListBox = NULL;
HWND m_hWndPlaylistSearchTextBox = NULL;

wchar_t *lpwcQuickPlaylist = NULL;
wchar_t *lpwcSongSearch = NULL;

HANDLE g_hSS_Stop = NULL;
HANDLE g_hSS_Stopped = NULL;
CRITICAL_SECTION g_csSongSearch;