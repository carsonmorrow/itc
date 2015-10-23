#ifndef __GLOBALDATA_H__INCLUDED
#define __GLOBALDATA_H__INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <string>

class CITunesCOM;
struct SettingsStruct;
class InputWindow;
class SysTray;
class GraphicsInterface;

extern HWND m_hWndInput;
extern HWND m_hWndPlaylistSearch;
extern HWND m_hWndSongSearch;
extern SettingsStruct m_ssSettings;
extern bool m_bTrackChangeAllowed;

extern CITunesCOM *m_ITCom;
extern InputWindow *inputWindow;
extern std::vector<std::wstring> vPlaylists;
extern GraphicsInterface *_ginterface;

extern HINSTANCE g_hInstance;
extern SysTray st;

extern HANDLE hPlaylistSearchThread;
extern HANDLE hSongSearchThread;

extern wchar_t *lpwcQuickPlaylist;
extern wchar_t *lpwcSongSearch;

extern HANDLE g_hSS_Stop;
extern HANDLE g_hSS_Stopped;
extern CRITICAL_SECTION g_csSongSearch;

extern HWND m_hWndSongSearchListBox;
extern HWND m_hWndSongSearchTextBox;
extern HWND m_hWndPlaylistSearchListBox;
extern HWND m_hWndPlaylistSearchTextBox;

//#define STRING2(x) #x
//#define STRING(x) STRING2(x)

#endif