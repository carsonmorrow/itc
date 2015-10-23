#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <errno.h>

#include <string>
#include <vector>
using namespace std;

#include "../common/pugixml.hpp"
#ifndef NO_HOTKEYS_SUPPORT
#include "../kbhook/kbhook.h"
#endif

#define MAJORNUMBER 0
#define MINORNUMBER 62
#define BUILDNUMBER 133
#define PRETTYVERSION L"0.63"
#define BUILDVERSION L"134"
#define NUMERICVERSION L"0.63.134"

#define WCONFSTR L"<itc><settings /><hotkeys /></itc>"

struct HotKeyItem
{
	UINT16 uiKey;
	UINT8 uiMods;
	UINT8 nId;
	wchar_t wcSettingsName[30];
	wchar_t wcPrintName[30];
};

struct SettingsStruct
{
	bool bPromptQuitiTunes;
	bool bQuitiTunesOnExit;
	bool bStartiTunesAtStart;
	bool bStartWithWindows;
	bool bStartWithiTunes;
	bool bStartOnHotkeys;
	bool bShowInTray;
	bool bVisualFeedback;
	bool bMinimizeiTunesAtStart;
	bool bQuitWithiTunes;
	bool bShowCfgSaveWarning;
	bool bShowDisplayAtEnd;
	bool bShowDisplayAtStart;
	bool bNoDisplayShortTracks;
	bool bAllowBlankLines;
	int nTrackChangeDelay;
	int nPopupShowTime;
	int nDisplayEndTime;
	int nDisplayStartTime;
	int nShortTrackLength;
	int nShowBeginOffset;
	int nShowEndOffset;
	int nVolumeChgInt;
	int nOSDVert;
	int nOSDHoriz;
	int nSleepTimer;
	int nSleepType; //0 - pause itunes, 1 - close itunes, 2 - shutdown computer
	int nTagField; //0 - comment (default), 1 - grouping
	int nJumpOneType;
	int nJumpTwoType;
	int nJumpOneValue;
	int nJumpTwoValue;
	bool bPromptOnDelete;
	bool bDeleteFromDisk;
	bool bDisableACOnFocus;

	int nMaxSongSearchResults;
	bool bAlwaysSearchLibrary;
	int nSSEnterAction;
	int nSSShiftEnterAction;
	int nSSCtrlEnterAction;

	long lOSDColorRef;
	long lOSDBkColorRef;
	long lOSDBorderColor;
	long lArtBorderColor;
	long lDropShadowColor;
	int nOSDBorderSize;
	int nArtBorderSize;
	int nOSDFontPoint;
	int nOSDFontStyle;
	int nOSDMonitor;
	bool bOSDAllMonitors;

	int nDisplayLayout;
	bool bUseDropShadow;
	float fDropShadowOffset;
	bool bNoDisplayWhenActive;
	bool bOSDTopMost;
	bool bOSDShowMissingTags;
	bool bOSDAlwaysUp;
	bool bOSDHideWhenPaused;
	bool bOSDOutlineMode;
	bool bShowArtwork;
	bool bFolderArt;
	bool bStaticArt;
	wchar_t wcStaticArtPath[MAX_PATH];
	bool bOSDForceWidth;
	bool bOSDUseBorder;
	int nOSDMaxWidth;
	bool bFadeEffects;
	int nOSDTextJustify;
	int nArtConstSize;
	int nArtMode;
	int nOSDAlpha;
	int nRatingDelta;
	wchar_t wcFontFace[32];
	bool bOnlyToggleRepeatOne;

	wchar_t wcOSDFormat[512];
	wchar_t wciTunesTitle[64];
	wchar_t wcCSIFormat[128];
	wchar_t wcQuickPlaylist[64];

	int nRatingType;
	wchar_t wcRatingFullChar;
	wchar_t wcRatingHalfChar;
	wchar_t wcRatingEmptyChar;
	wchar_t wcRatingFullPath[MAX_PATH];
	wchar_t wcRatingHalfPath[MAX_PATH];
	wchar_t wcRatingEmptyPath[MAX_PATH];
	long lRatingShapeOutline;
	long lRatingShapeFill;
	int nRatingShapeStyle;
	int nRatingShapeType;
};

void LoadSettings(vector<HotKeyItem> *vKeys, SettingsStruct *ssSettings);
HRESULT LoadHotkey(const pugi::xml_node& node, const wchar_t* lpwcName, HotKeyItem *phkiItem, UINT8 nDefMod, UINT16 nDefKey);
HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, bool & bValue, bool bDefault);
HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, int & nValue, int nDefault);
HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, __int64 & nValue, __int64 nDefault);
HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, long & lValue, long lDefault);
HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, float & fValue, float fDefault);
HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, wchar_t *wcValue, const wchar_t* wcDefault, size_t nMaxLength);
HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, char *cValue, const char* cDefault, size_t nMaxLength);

// New vectorized hotkey support
#define BEGIN_HOTKEYS_MAP(vk) { vector<HotKeyItem> *v=vk; HotKeyItem i = {0};
#define HOTKEY_ENTRY(id,key,mods,disp,save) v->push_back( FillHotKeyItem( i, id, key, mods, disp, save ) );
#define END_HOTKEYS_MAP() }
void CreateDefaultHotkeys( vector<HotKeyItem> *vKeys );
HotKeyItem& FillHotKeyItem( HotKeyItem &hkiHotKey, UINT8 nId, UINT16 uiKey, UINT8 uiMods, const wchar_t* wcPrintName, const wchar_t* wcSettingsName );

// Functions only needed for saving data
void SaveSettings(vector<HotKeyItem> *vKeys, SettingsStruct *ssSettings);

HRESULT SaveHotkey(pugi::xml_node& node, const wchar_t* lpwcName, HotKeyItem hkiItem);
HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, bool bValue);
HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, int nValue);
HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, __int64 nValue);
HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, long lValue);
HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, float fValue);
HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, const wchar_t* wcValue);
HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, const char* cValue);

bool IsPortable();

#ifndef NO_HOTKEYS_SUPPORT
void ApplyHotKeys(vector<HotKeyItem> *vKeys);
void UnApplyHotKeys();
#endif

// Hotkey ID List
#define HK_PLAYPAUSE 0
#define HK_PREV 1
#define HK_NEXT 2
#define HK_SHOWCUR 3
#define HK_QUIT 4
#define HK_NEXTARTIST 5
#define HK_PREVARTIST 6
#define HK_NEXTALBUM 7
#define HK_PREVALBUM 8
#define HK_VOLUMEUP 9
#define HK_VOLUMEDOWN 10
#define HK_VOLUMEMUTE 11
#define HK_SHUFFLE 12
#define HK_RATEZERO 13
#define HK_RATEONE 14
#define HK_RATETWO 15
#define HK_RATETHREE 16
#define HK_RATEFOUR 17
#define HK_RATEFIVE 18
#define HK_SHOWRATE 19
#define HK_REPEATMODE 20
#define HK_FASTFORWARD 21
#define HK_REWIND 22
#define HK_SHOWHIDE 23
#define HK_SHOWSETTINGS 25
#define HK_SLEEPTIMER 26
#define HK_SETRATING 27
#define HK_RATINGINC 28
#define HK_RATINGDEC 29
#define HK_UPDATEIPOD 30
#define HK_COPYSONGINFO 31
#define HK_TAG 32
#define HK_QUICKPLAYLIST 33
#define HK_NORMALIZEUP 34
#define HK_NORMALIZEDOWN 35
#define HK_NORMALIZEZERO 36
#define HK_PLAYLISTSEARCH 37
#define HK_JUMPONE 38
#define HK_JUMPTWO 39
#define HK_STOP 40
#define HK_SONGSEARCH 41
#define HK_STOPAFTERCURRENT 42
#define HK_TOGGLEENABLE 43
#define HK_PLAYINITUNESDJ 44
#define HK_SHOWHIDEDISPLAY 45
#define HK_DELETE 47
#define HK_RESTARTTRACK 48

#define WM_CSETHOTKEY WM_APP + 7
#define WM_CGETHOTKEY WM_APP + 8
