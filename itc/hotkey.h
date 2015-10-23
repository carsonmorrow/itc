#ifndef __HOTKEYS_H__INCLUDED
#define __HOTKEYS_H__INCLUDED

#include <windows.h>
#include <tchar.h>

#include "globaldata.h"
#include "com.h"
#include "searchhandlers.h"
#include "settings.h"

LRESULT ProcessHotkey(HWND hWnd, WPARAM wParam);

bool ShowAddTags();
bool ShowAllTags();
void DoTagStuff();
void DoOtherTagStuff();

extern const int WM_STARTWITHITUNES;

#endif