#pragma once

#ifdef KBHOOK_EXPORTS
#define KBHOOK_API __declspec(dllexport)
#else
#define KBHOOK_API __declspec(dllimport)
#endif

#define HWM_HOOKMSG_STR L"HWM_HOOKMSG_STR-{99F8D180-599E-40A3-865F-9149C20823EF}"

#define MOD_APPCOMMAND 0x10

KBHOOK_API void RegisterListener( HWND hwndListener );
KBHOOK_API void UnregisterListener( void );
KBHOOK_API void SetMatchMode( bool bMatchCombos );
KBHOOK_API void ClearCombos( void );
KBHOOK_API void AddCombo( UINT8 uiMod, UINT16 uiKey, UINT8 uiId );
KBHOOK_API void SetACIgnore(bool bIgnoreACOnFocus, HWND hwndiTunes);