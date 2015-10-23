#ifndef __SEARCHHANDLERS_H__INCLUDED
#define __SEARCHHANDLERS_H__INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "globaldata.h"
#include "com.h"

DWORD WINAPI PlaylistSearchThreadProc(LPVOID lpParameter);
LRESULT CALLBACK PlaylistSearchTextBoxProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );

DWORD WINAPI SongSearchThreadProc(LPVOID lpParameter);
LRESULT CALLBACK SongSearchTextBoxProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
LRESULT CALLBACK SongSearchListBoxProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );

LRESULT CALLBACK PlaylistSearchWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SongSearchWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

const int WM_QUICKPLAYLIST = WM_APP + 8;
const int WM_PLAYSSRESULT = WM_APP + 11;
const int WM_SONGSEARCH = WM_APP + 12;
const int WM_QUITTHREAD = WM_APP + 13;
#endif