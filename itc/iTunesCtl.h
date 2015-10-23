//iTunesControl
//Born: 2004-11-10

#ifndef ITUNESCTL_H__INCLUDED
#define ITUNESCTL_H__INCLUDED

#include "itunescominterface.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#include <string>

#ifndef _DEBUG
#define _STABLE
#endif

#include "globaldata.h"
#include "com.h"
#include "resource.h"
#include "core.h"
#include "settings.h"
#include "GraphicsInterface.h"
#include "InputEngine.h"
#include "../common/multimon.h"
#include "searchhandlers.h"
#include "hotkey.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ProcessCommandLine( HWND hWnd );
void SongSearchAction( int nType );

long _lTrackDBID, _lTrackID;
short _sShift;
short _sCtrl;
ULONG_PTR _gdiplusToken;

const int WM_ICONTRAY = WM_APP + 1;
const int WM_CONNECT = WM_APP + 4;
static const UINT WM_RELOADSETTINGS = RegisterWindowMessage( L"RELOAD_SETTINGS-{B77A1ADA-6421-45c8-A1E1-9B99D85E1486}" );
static const UINT WM_HOTKEYSACTION = RegisterWindowMessage( L"HOTKEYS_ACTION-{F44C184E-A088-4b7f-A24E-90CA8C68FE1B}" );
static const UINT WM_ITCSHUTDOWN = RegisterWindowMessage(L"ITC_SHUTDOWN-{2E8B5999-49A9-4205-95DF-D9C9A07B9816}");
static const UINT HWM_HOOKMSG = RegisterWindowMessage( HWM_HOOKMSG_STR );
extern const int WM_QUICKPLAYLIST;
const int WM_STARTWITHITUNES = WM_APP + 9;
const int WM_EXTHOTKEY = WM_APP + 10;
extern const int WM_PLAYSSRESULT;
extern const int WM_SONGSEARCH;

#define DISPLAY_UPDATE_PERIOD 500

#endif
