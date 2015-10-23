// kbhook.cpp : Defines the exported functions for the DLL application.
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "kbhook.h"

// ---- Defs ----
struct sHotkey
{
	UINT uiKey;
	bool bWin;
	bool bShift;
	bool bControl;
	bool bAlt;
	bool bSingle;
};

struct sCombo
{
	UINT16 uiKey;
	UINT8 uiMod;
	UINT8 uiId;
};

LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ShellProc( int nCode, WPARAM wParam, LPARAM lParam );
bool ValidateHotkey( WPARAM wParam, LPARAM lParam, sHotkey *shkHotkey );

#define IsKeyDownAsync(vk) (GetAsyncKeyState(vk) & 0x8000)

#define MAX_HOTKEYS 48

// ---- Shared global data ----
#pragma data_seg (".shared")
UINT g_nRefCount = 0;
HHOOK g_hKBHook = NULL;
HHOOK g_hSHook = NULL;
HWND g_hwndListener = NULL;
sHotkey g_shkBuilding = {0};
sCombo g_sCombos[MAX_HOTKEYS] = {0};
UINT g_uiCombo = 0;
bool g_bMatchCombos = false;
bool g_bSuppressKeyUp = false;
bool g_bIgnoreACOnFocus = false;
HWND g_hwndiTunes = NULL;
#pragma data_seg ()
#pragma comment ( linker, "/SECTION:.shared,RWS" )

// ---- Process global data ----
HINSTANCE g_hInstance = NULL;
UINT HWM_HOOKMSG = 0;

// ---- Code ----
BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls( hModule );
		// Keep a reference count so that we don't unhook too soon
		InterlockedIncrement( (volatile LONG *)&g_nRefCount );
		HWM_HOOKMSG = RegisterWindowMessage( HWM_HOOKMSG_STR );
		g_hInstance = hModule;
		break;

	case DLL_PROCESS_DETACH:
		InterlockedDecrement( (volatile LONG *)&g_nRefCount );
		if ( 0 == g_nRefCount )
		{
			UnhookWindowsHookEx( g_hKBHook );
			g_hKBHook = NULL;
			UnhookWindowsHookEx( g_hSHook );
			g_hSHook = NULL;
		}
		break;
	}
	return TRUE;
}

// Create hooks and associate with specified window
KBHOOK_API void RegisterListener( HWND hwndListener )
{
	if ( NULL == g_hKBHook )
		g_hKBHook = SetWindowsHookEx( WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, g_hInstance, 0 );

	if ( NULL == g_hSHook )
		g_hSHook = SetWindowsHookEx( WH_SHELL, (HOOKPROC)ShellProc, g_hInstance, 0 );

	g_hwndListener = hwndListener;

	return;
}

// Destroy hooks
KBHOOK_API void UnregisterListener( void )
{
	if ( NULL != g_hKBHook )
		UnhookWindowsHookEx( g_hKBHook );
	g_hKBHook = NULL;

	if ( NULL != g_hSHook )
		UnhookWindowsHookEx( g_hSHook );
	g_hSHook = NULL;

	return;
}

KBHOOK_API void SetMatchMode( bool bMatchCombos )
{
	g_bMatchCombos = bMatchCombos;

	return;
}

KBHOOK_API void ClearCombos( void )
{
	g_uiCombo = 0;

	return;
}

KBHOOK_API void AddCombo( UINT8 uiMod, UINT16 uiKey, UINT8 uiId )
{
	if ( g_uiCombo > (MAX_HOTKEYS-1) )
		return;

	g_sCombos[g_uiCombo].uiId = uiId;
	g_sCombos[g_uiCombo].uiKey = uiKey;
	g_sCombos[g_uiCombo].uiMod = uiMod;

	++g_uiCombo;

	return;
}

void SetACIgnore(bool bIgnoreACOnFocus, HWND hwndiTunes)
{
	g_bIgnoreACOnFocus = bIgnoreACOnFocus;
	g_hwndiTunes = hwndiTunes;
}

// Process WM_APPCOMMAND messages
// This assumes our hook gets processed before the hooks of anyone else
//   who might use the messages
LRESULT CALLBACK ShellProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	if ( HSHELL_APPCOMMAND == nCode )
	{
		if (g_bIgnoreACOnFocus && g_hwndiTunes == GetForegroundWindow())
		{
			return 0;
		}

		if ( g_bMatchCombos )
		{
			// Search through combos for match
			for ( UINT i = 0; i < g_uiCombo; i++ )
			{
				if ( g_sCombos[i].uiKey == GET_APPCOMMAND_LPARAM(lParam) && g_sCombos[i].uiMod == MOD_APPCOMMAND )
				{
					PostMessage( g_hwndListener, HWM_HOOKMSG, g_sCombos[i].uiId, 0 );
					return TRUE;
				}
			}
			// If we get here, we didn't match anything, so just let WM_APPCOMMAND keep going
		}
		else
		{
			PostMessage( g_hwndListener, HWM_HOOKMSG, MOD_APPCOMMAND, GET_APPCOMMAND_LPARAM(lParam) );

			return TRUE;
		}
	}

	return 0;
}

LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	// MSDN says to ignore anything that's not HC_ACTION
	if ( HC_ACTION != nCode )
		return CallNextHookEx( g_hKBHook, nCode, wParam, lParam );

	KBDLLHOOKSTRUCT &kbevent = *(PKBDLLHOOKSTRUCT)lParam;

	// We inject Ctrl presses sometimes, so we shouldn't match on those
	if ( ( kbevent.flags & LLKHF_INJECTED ) && ( kbevent.vkCode == VK_LCONTROL || wParam == VK_RCONTROL ) )
		return CallNextHookEx( g_hKBHook, nCode, wParam, lParam );

	// iTunes will probably always get WM_APPCOMMANDs before us, leaving us with the VKs for the keyboard media keys
	// If we want to ignore the keyboard's media keys when iTunes has focus, we have to do it here, too
	if ((kbevent.vkCode >= VK_VOLUME_MUTE && kbevent.vkCode <= VK_MEDIA_PLAY_PAUSE) && g_bIgnoreACOnFocus && g_hwndiTunes == GetForegroundWindow())
		return CallNextHookEx( g_hKBHook, nCode, wParam, lParam );

	// We won't necessarily get any key ups from Ctrl + Alt + Del or Win + L, so just filter out now
	if ( VK_DELETE == kbevent.vkCode && g_shkBuilding.bControl && g_shkBuilding.bAlt && !g_shkBuilding.bShift && !g_shkBuilding.bWin )
	{
		memset( &g_shkBuilding, 0, sizeof(sHotkey) );
		return CallNextHookEx( g_hKBHook, nCode, wParam, lParam );
	}
	if ( 'L' == kbevent.vkCode && g_shkBuilding.bWin && !g_shkBuilding.bControl && !g_shkBuilding.bAlt && !g_shkBuilding.bShift )
	{
		memset( &g_shkBuilding, 0, sizeof(sHotkey) );
		return CallNextHookEx( g_hKBHook, nCode, wParam, lParam );
	}

	if ( WM_KEYDOWN == wParam || WM_SYSKEYDOWN == wParam )
	{
		// Either match mode, shkBuilding needs to be updated.
		if ( g_bMatchCombos )
		{
			UINT uiMod = 0;
			g_bSuppressKeyUp = false;

			// Grab hotkey data to sHotkey
			if ( kbevent.vkCode == VK_LWIN || kbevent.vkCode == VK_RWIN )
				g_shkBuilding.bWin = true;
			else if ( kbevent.vkCode == VK_LSHIFT || kbevent.vkCode == VK_RSHIFT || kbevent.vkCode == VK_SHIFT )
				g_shkBuilding.bShift = true;
			else if ( kbevent.vkCode == VK_LCONTROL || kbevent.vkCode == VK_RCONTROL || kbevent.vkCode == VK_CONTROL )
				g_shkBuilding.bControl = true;
			else if ( VK_LMENU == kbevent.vkCode || VK_RMENU == kbevent.vkCode || kbevent.vkCode == VK_MENU )
				g_shkBuilding.bAlt = true;
			else
				g_shkBuilding.uiKey = kbevent.vkCode;

			// Convert mods into format that matches sCombo
			if (g_shkBuilding.bControl)
				uiMod |= MOD_CONTROL;
			if (g_shkBuilding.bShift)
				uiMod |= MOD_SHIFT;
			if (g_shkBuilding.bWin)
				uiMod |= MOD_WIN;
			if (g_shkBuilding.bAlt)
				uiMod |= MOD_ALT;

			// Treat all hotkeys as [modifiers][key], so only look for matches
			//   on non-modifier key presses
			if ( kbevent.vkCode && kbevent.vkCode == g_shkBuilding.uiKey )
			{
				// Search through combos for match
				for ( UINT i = 0; i < g_uiCombo; i++ )
				{
					if ( g_sCombos[i].uiKey == g_shkBuilding.uiKey && g_sCombos[i].uiMod == uiMod )
					{
						g_bSuppressKeyUp = true;
						PostMessage( g_hwndListener, HWM_HOOKMSG, g_sCombos[i].uiId, 0 );

						// "Suppress" Win or Alt by faking Control press
						if ( uiMod == MOD_ALT || uiMod == MOD_WIN )
						{
							// We don't need to send Ctrl press if it's being done for us
							if ( !( IsKeyDownAsync( VK_LCONTROL ) || IsKeyDownAsync( VK_RCONTROL ) || IsKeyDownAsync( VK_CONTROL ) ) )
							{
								// Send Ctrl up/down to "suppress"
								keybd_event( VK_CONTROL, MapVirtualKey( VK_CONTROL, MAPVK_VK_TO_VSC ), KEYEVENTF_EXTENDEDKEY, 0 );
								keybd_event( VK_CONTROL, MapVirtualKey( VK_CONTROL, MAPVK_VK_TO_VSC ), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
							}
						}
						return 1;
					}
				}
			}
		}
		// Match-all mode, used by config utility
		else
		{
			// Using the data we got, make something that looks like the WM_KEY--- LPARAM
			LPARAM lFakeParam = kbevent.scanCode << 16 | ( kbevent.flags & 0x1 ) << 24 | ( kbevent.flags & 0x20 ) << 29;

			if ( ValidateHotkey( kbevent.vkCode, lFakeParam, &g_shkBuilding ) )
			{
				UINT uiMod = 0;

				if (g_shkBuilding.bControl)
					uiMod |= MOD_CONTROL;
				if (g_shkBuilding.bShift)
					uiMod |= MOD_SHIFT;
				if (g_shkBuilding.bWin)
					uiMod |= MOD_WIN;
				if (g_shkBuilding.bAlt)
					uiMod |= MOD_ALT;

				PostMessage( g_hwndListener, HWM_HOOKMSG, uiMod, g_shkBuilding.uiKey );

				return 1;
			}
		}
	}
	// Key up processing
	else
	{
		if ( kbevent.vkCode == VK_LWIN || kbevent.vkCode == VK_RWIN )
			g_shkBuilding.bWin = false;
		else if ( kbevent.vkCode == VK_LSHIFT || kbevent.vkCode == VK_RSHIFT || kbevent.vkCode == VK_SHIFT )
			g_shkBuilding.bShift = false;
		else if ( kbevent.vkCode == VK_LCONTROL || kbevent.vkCode == VK_RCONTROL || kbevent.vkCode == VK_CONTROL )
			g_shkBuilding.bControl = false;
		else if ( VK_LMENU == kbevent.vkCode || VK_RMENU == kbevent.vkCode || kbevent.vkCode == VK_MENU )
			g_shkBuilding.bAlt = false;
		else if ( kbevent.vkCode && g_shkBuilding.uiKey == kbevent.vkCode )
		{
			g_shkBuilding.uiKey = 0;
			g_shkBuilding.bSingle = false;
		}

		// If we got a combo match, we don't want suppress certain key up events
		if ( g_bSuppressKeyUp )
		{
			// Continue propagating modifiers since we let the keydown event through
			if ( kbevent.vkCode == VK_LWIN || kbevent.vkCode == VK_RWIN ||
				VK_LMENU == kbevent.vkCode || VK_RMENU == kbevent.vkCode || VK_MENU == kbevent.vkCode ||
				kbevent.vkCode == VK_LCONTROL || kbevent.vkCode == VK_RCONTROL || kbevent.vkCode == VK_CONTROL || 
				kbevent.vkCode == VK_LSHIFT || kbevent.vkCode == VK_RSHIFT || kbevent.vkCode == VK_SHIFT)
			{
				return CallNextHookEx( g_hKBHook, nCode, wParam, lParam );
			}
			// Everything else would be the actual non-modifier key -- the key down should have been suppressed, so key up should be, too
			else
				return 1;
		}
	}

	return CallNextHookEx( g_hKBHook, nCode, wParam, lParam );
}

bool ValidateHotkey( WPARAM wParam, LPARAM lParam, sHotkey *shkHotkey )
{
	if ( wParam == VK_LWIN || wParam == VK_RWIN )
	{
		shkHotkey->bWin = true;
	}
	else if ( wParam == VK_LSHIFT || wParam == VK_RSHIFT || wParam == VK_SHIFT )
	{
		shkHotkey->bShift = true;
	}
	else if ( wParam == VK_LCONTROL || wParam == VK_RCONTROL || wParam == VK_CONTROL )
	{
		shkHotkey->bControl = true;
	}
	else if ( VK_LMENU == wParam || VK_RMENU == wParam || VK_MENU == wParam )
	{
		shkHotkey->bAlt = true;
	}
	else if ( VK_PAUSE == wParam || VK_CANCEL == wParam || ( wParam >= VK_F1 && wParam <= VK_F24 ) ||
		( wParam >= VK_BROWSER_BACK && wParam <= VK_LAUNCH_APP2 ) || wParam == VK_PRIOR || wParam == VK_NEXT)
	{
		shkHotkey->uiKey = wParam;
		shkHotkey->bSingle = true;
	}
	else
	{
		shkHotkey->uiKey = wParam;
	}

	if ( (shkHotkey->bWin || shkHotkey->bShift || shkHotkey->bAlt || shkHotkey->bControl || shkHotkey->bSingle) && shkHotkey->uiKey != 0 )
		return true;
	else
		return false;
}