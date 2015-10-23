#include "settings.h"
#include "memcheck.h"
#include <tchar.h>

#ifndef NO_HOTKEYS_SUPPORT
void ApplyHotKeys(vector<HotKeyItem> *vKeys)
{
	vector<HotKeyItem>::iterator p = vKeys->begin();

	while (p != vKeys->end())
	{
		if (p->uiKey != 0)
			AddCombo(p->uiMods, p->uiKey, p->nId);

		++p;
	}

	return;
}

void UnApplyHotKeys()
{
	ClearCombos();

	return;
}
#endif

HRESULT LoadHotkey(const pugi::xml_node& node, const wchar_t* lpwcName, HotKeyItem *phkiItem, UINT8 nDefMod, UINT16 nDefKey)
{
	pugi::xml_node child = node.child(lpwcName);

	if (child)
	{
		phkiItem->uiMods = (UINT8)child.attribute(L"mod").as_uint();
		phkiItem->uiKey = (UINT16)child.attribute(L"key").as_uint();
	}
	else
	{
		phkiItem->uiMods = nDefMod;
		phkiItem->uiKey = nDefKey;
	}

	return S_OK;
}

HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, bool & bValue, bool bDefault)
{
	pugi::xml_node child = node.child(lpwcName);

	if (child)
		bValue = child.attribute(L"value").as_bool();
	else
		bValue = bDefault;

	return S_OK;
}

HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, __int64 & nValue, __int64 nDefault)
{
	pugi::xml_node child = node.child(lpwcName);

	if (child)
		nValue = _wtoi64(child.attribute(L"value").value());
	else
		nValue = nDefault;

	return S_OK;
}

HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, int & nValue, int nDefault)
{
	pugi::xml_node child = node.child(lpwcName);

	if (child)
		nValue = child.attribute(L"value").as_int();
	else
		nValue = nDefault;

	return S_OK;
}

HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, long & lValue, long lDefault)
{
	pugi::xml_node child = node.child(lpwcName);

	if (child)
		lValue = _wtol(child.attribute(L"value").value());
	else
		lValue = lDefault;

	return S_OK;
}

HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, float & fValue, float fDefault)
{
	pugi::xml_node child = node.child(lpwcName);

	if (child)
		fValue = child.attribute(L"value").as_float();
	else
		fValue = fDefault;

	return S_OK;
}

HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, wchar_t *wcValue, const wchar_t* wcDefault, size_t nMaxLength)
{
	pugi::xml_node child = node.child(lpwcName);

	if (child)
		wcscpy_s(wcValue, nMaxLength, child.attribute(L"value").value());
	else
		wcscpy_s(wcValue, nMaxLength, wcDefault);

	return S_OK;
}

HRESULT LoadSetting(const pugi::xml_node& node, const wchar_t* lpwcName, char *cValue, const char* cDefault, size_t nMaxLength)
{
	pugi::xml_node child = node.child(lpwcName);
	size_t chars_conv = 0;
	char *cTemp = NULL;

	if (child)
	{
		wcstombs_s(&chars_conv, NULL, 0, child.attribute(L"value").value(), _TRUNCATE);
		cTemp = new char[chars_conv];
		wcstombs_s(&chars_conv, cTemp, chars_conv, child.attribute(L"value").value(), _TRUNCATE);
		strcpy_s(cValue, nMaxLength, cTemp);
		delete[] cTemp;
	}
	else
		strcpy_s(cValue, nMaxLength, cDefault);

	return S_OK;
}

HotKeyItem& FillHotKeyItem(HotKeyItem &hkiHotKey, UINT8 nId, UINT16 uiKey, UINT8 uiMods, const wchar_t* wcPrintName, const wchar_t* wcSettingsName)
{
	hkiHotKey.nId = nId;
	hkiHotKey.uiKey = uiKey;
	hkiHotKey.uiMods = uiMods;
	wcscpy_s(hkiHotKey.wcPrintName, 30, wcPrintName);
	wcscpy_s(hkiHotKey.wcSettingsName, 30, wcSettingsName);

	return hkiHotKey;
}

void CreateDefaultHotkeys(vector<HotKeyItem> *vKeys)
{
	BEGIN_HOTKEYS_MAP(vKeys)
		HOTKEY_ENTRY(HK_PLAYPAUSE, 'P', MOD_SHIFT | MOD_CONTROL, L"Play/Pause", L"PlayPause")
		HOTKEY_ENTRY(HK_PREV, 37, MOD_WIN | MOD_CONTROL, L"Previous Track", L"PrevTrack")
		HOTKEY_ENTRY(HK_NEXT, 39, MOD_WIN | MOD_CONTROL, L"Next Track", L"NextTrack")
		HOTKEY_ENTRY(HK_STOP, 0, 0, L"Stop", L"Stop")
		HOTKEY_ENTRY(HK_VOLUMEUP, 38, MOD_WIN | MOD_CONTROL, L"Volume Up", L"VolumeUp")
		HOTKEY_ENTRY(HK_VOLUMEDOWN, 40, MOD_WIN | MOD_CONTROL, L"Volume Down", L"VolumeDown")
		HOTKEY_ENTRY(HK_VOLUMEMUTE, 40, MOD_WIN | MOD_SHIFT, L"Volume Mute", L"VolumeMute")
		HOTKEY_ENTRY(HK_QUIT, 'Q', MOD_WIN, L"Quit", L"Quit")
		HOTKEY_ENTRY(HK_SHOWCUR, 'C', MOD_WIN, L"Show/Hide Display", L"ShowCurrent")
		HOTKEY_ENTRY(HK_NEXTARTIST, 'A', MOD_WIN, L"Next Artist", L"NextArtist")
		HOTKEY_ENTRY(HK_PREVARTIST, 'A', MOD_WIN | MOD_SHIFT, L"Previous Artist", L"PrevArtist")
		HOTKEY_ENTRY(HK_NEXTALBUM, 'Z', MOD_WIN, L"Next Album", L"NextAlbum")
		HOTKEY_ENTRY(HK_PREVALBUM, 'Z', MOD_WIN | MOD_SHIFT, L"Previous Album", L"PrevAlbum")
		HOTKEY_ENTRY(HK_SHUFFLE, 'S', MOD_WIN, L"Toggle Shuffle", L"ToggleShuffle")
		HOTKEY_ENTRY(HK_RATEZERO, 0, 0, L"Rate Zero", L"RateZero")
		HOTKEY_ENTRY(HK_RATEONE, 0, 0, L"Rate One", L"RateOne")
		HOTKEY_ENTRY(HK_RATETWO, 0, 0, L"Rate Two", L"RateTwo")
		HOTKEY_ENTRY(HK_RATETHREE, 0, 0, L"Rate Three", L"RateThree")
		HOTKEY_ENTRY(HK_RATEFOUR, 0, 0, L"Rate Four", L"RateFour")
		HOTKEY_ENTRY(HK_RATEFIVE, 0, 0, L"Rate Five", L"RateFive")
		HOTKEY_ENTRY(HK_SHOWRATE, 0, 0, L"Show Rating", L"ShowRate")
		HOTKEY_ENTRY(HK_REPEATMODE, 'J', MOD_WIN, L"Repeat Mode", L"RepeatMode")
		HOTKEY_ENTRY(HK_FASTFORWARD, 39, MOD_WIN | MOD_SHIFT, L"Fast Forward", L"FastForward")
		HOTKEY_ENTRY(HK_REWIND, 37, MOD_WIN | MOD_SHIFT, L"Rewind", L"Rewind")
		HOTKEY_ENTRY(HK_SHOWHIDE, 'S', MOD_WIN | MOD_SHIFT, L"Show/Hide iTunes", L"ShowHide")
		HOTKEY_ENTRY(HK_SHOWSETTINGS, 'S', MOD_WIN | MOD_SHIFT | MOD_CONTROL, L"Show Settings", L"ShowSettings")
		HOTKEY_ENTRY(HK_SLEEPTIMER, 'X', MOD_WIN | MOD_SHIFT, L"Sleep Timer", L"SleepTimer")
		HOTKEY_ENTRY(HK_SETRATING, 0, 0, L"Set Rating", L"SetRating")
		HOTKEY_ENTRY(HK_RATINGINC, 0, 0, L"Rating Increment", L"RatingInc")
		HOTKEY_ENTRY(HK_RATINGDEC, 0, 0, L"Rating Decrement", L"RatingDec")
		HOTKEY_ENTRY(HK_UPDATEIPOD, 0, 0, L"Update iPod", L"UpdateIPod")
		HOTKEY_ENTRY(HK_COPYSONGINFO, 0, 0, L"Copy Song Info", L"CopySongInfo")
		HOTKEY_ENTRY(HK_TAG, 0, 0, L"Tag", L"Tag")
		HOTKEY_ENTRY(HK_QUICKPLAYLIST, 0, 0, L"Quick Playlist", L"QuickPlaylist")
		HOTKEY_ENTRY(HK_NORMALIZEUP, 0, 0, L"Normalize Up", L"NormalizeUp")
		HOTKEY_ENTRY(HK_NORMALIZEDOWN, 0, 0, L"Normalize Down", L"NormalizeDown")
		HOTKEY_ENTRY(HK_NORMALIZEZERO, 0, 0, L"Normalize Zero", L"NormalizeZero")
		HOTKEY_ENTRY(HK_PLAYLISTSEARCH, 0, 0, L"Playlist Search", L"PlaylistSearch")
		HOTKEY_ENTRY(HK_JUMPONE, 0, 0, L"Short Jump", L"JumpOne")
		HOTKEY_ENTRY(HK_JUMPTWO, 0, 0, L"Long Jump", L"JumpTwo")
		HOTKEY_ENTRY(HK_SONGSEARCH, 0, 0, L"Song Search", L"SongSearch")
		HOTKEY_ENTRY(HK_STOPAFTERCURRENT, 0, 0, L"Stop After Current Track", L"StopAfterCurrent");
	HOTKEY_ENTRY(HK_TOGGLEENABLE, 0, 0, L"Toggle Track Enable", L"ToggleTrackEnable");
	HOTKEY_ENTRY(HK_PLAYINITUNESDJ, 0, 0, L"Play in iTunes DJ", L"PlayiniTunesDJ");
	HOTKEY_ENTRY(HK_SHOWHIDEDISPLAY, 0, 0, L"Show/Hide Display", L"ShowHideDisplay");
	HOTKEY_ENTRY(HK_DELETE, 0, 0, L"Delete Current Track", L"DeleteCurrentTrack");
	HOTKEY_ENTRY(HK_RESTARTTRACK, 0, 0, L"Restart Current Track", L"RestartCurrentTrack");
	END_HOTKEYS_MAP()
}

bool IsPortable()
{
	wchar_t wcFilename[MAX_PATH];
	struct _stat buf;

	GetModuleFileName(NULL, wcFilename, MAX_PATH);
	PathRemoveFileSpec(wcFilename);
	PathAppend((LPWSTR)wcFilename, L"config.xml");

	// Assume that an error means no file
	if (0 != _wstat(wcFilename, &buf))
		return false;
	else
		return true;
}

void LoadSettings(vector<HotKeyItem> *vKeys, SettingsStruct *ssSettings)
{
	wchar_t szPath[MAX_PATH];
	if (!IsPortable())
	{
		if (SUCCEEDED(SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, L"iTunesControl", szPath)))
			PathAppend(szPath, L"config.xml");
	}
	else
	{
		GetModuleFileName(NULL, szPath, MAX_PATH);
		PathRemoveFileSpec(szPath);
		PathAppend((LPWSTR)szPath, L"config.xml");
	}

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(szPath);

	if (result.status == pugi::status_file_not_found)
	{
		if (!doc.load(WCONFSTR))
		{
			MessageBox(NULL, L"Fatal error while attempting to load settings!", L"Fatal Error", MB_OK | MB_ICONHAND);
			exit(-1);
		}
	}
	else if (result.status != pugi::status_ok)
	{
		MessageBox(NULL, L"Fatal error while attempting to load settings!", L"Fatal Error", MB_OK | MB_ICONHAND);
		exit(-1);
	}

	pugi::xml_node root = doc.child(L"itc");
	pugi::xml_node node = root.child(L"hotkeys");
	//Hotkeys
	if (vKeys)
	{
		vKeys->clear();
		CreateDefaultHotkeys(vKeys);

		vector<HotKeyItem>::iterator p = vKeys->begin();

		while (p != vKeys->end())
		{
			LoadHotkey(node, p->wcSettingsName, &(*p), p->uiMods, p->uiKey);

			++p;
		}
	}

	node = root.child(L"settings");

	//General settings
	if (ssSettings)
	{
		LoadSetting(node, L"PromptQuitiTunes", ssSettings->bPromptQuitiTunes, true);
		LoadSetting(node, L"QuitiTunesOnExit", ssSettings->bQuitiTunesOnExit, true);
		LoadSetting(node, L"StartiTunesAtStart", ssSettings->bStartiTunesAtStart, true);
		LoadSetting(node, L"StartWithWindows", ssSettings->bStartWithWindows, false);
		LoadSetting(node, L"QuitWithiTunes", ssSettings->bQuitWithiTunes, true);
		LoadSetting(node, L"StartWithiTunes", ssSettings->bStartWithiTunes, true);
		LoadSetting(node, L"StartOnHotkeys", ssSettings->bStartOnHotkeys, true);
		LoadSetting(node, L"MinimizeiTunesAtStart", ssSettings->bMinimizeiTunesAtStart, false);
		LoadSetting(node, L"ShowCfgSaveWarning", ssSettings->bShowCfgSaveWarning, true);
		ssSettings->nTrackChangeDelay = 150;
		ssSettings->nSleepTimer = 30;
		ssSettings->nSleepType = 0;
		LoadSetting(node, L"VolumeChangeInt", ssSettings->nVolumeChgInt, 5);
		LoadSetting(node, L"InfoShowTime", ssSettings->nPopupShowTime, 3000);
		LoadSetting(node, L"DisplayEndTime", ssSettings->nDisplayEndTime, 3000);
		LoadSetting(node, L"DisplayStartTime", ssSettings->nDisplayStartTime, 3000);
		LoadSetting(node, L"ShowBeginOffset", ssSettings->nShowBeginOffset, 0);
		LoadSetting(node, L"ShowEndOffset", ssSettings->nShowEndOffset, 0);
		LoadSetting(node, L"AllowBlankLines", ssSettings->bAllowBlankLines, false);
		LoadSetting(node, L"ShowDisplayAtEnd", ssSettings->bShowDisplayAtEnd, false);
		LoadSetting(node, L"ShowDisplayAtStart", ssSettings->bShowDisplayAtStart, true);
		LoadSetting(node, L"ShowInTray", ssSettings->bShowInTray, true);
		LoadSetting(node, L"iTunesTitle", ssSettings->wciTunesTitle, L"%track% - %artist%", 64);
		LoadSetting(node, L"CSIFormat", ssSettings->wcCSIFormat, L"Now Playing %artist% - %track% From The Album %album%", 128);
		LoadSetting(node, L"VisualFeedback", ssSettings->bVisualFeedback, true);
		LoadSetting(node, L"TagField", ssSettings->nTagField, 0);
		LoadSetting(node, L"QuickPlaylist", ssSettings->wcQuickPlaylist, L"Party Shuffle", 64);
		LoadSetting(node, L"JumpOneType", ssSettings->nJumpOneType, 0);
		LoadSetting(node, L"JumpTwoType", ssSettings->nJumpTwoType, 0);
		LoadSetting(node, L"JumpOneValue", ssSettings->nJumpOneValue, 0);
		LoadSetting(node, L"JumpTwoValue", ssSettings->nJumpTwoValue, 0);
		LoadSetting(node, L"RatingDelta", ssSettings->nRatingDelta, 20);
		LoadSetting(node, L"PromptOnDelete", ssSettings->bPromptOnDelete, true);
		LoadSetting(node, L"DeleteFromDisk", ssSettings->bDeleteFromDisk, true);
		LoadSetting(node, L"DisableACOnFocus", ssSettings->bDisableACOnFocus, false);
		LoadSetting(node, L"OnlyToggleRepeatOne", ssSettings->bOnlyToggleRepeatOne, false);

		// Song Search settings
		LoadSetting(node, L"MaxSongSearchResults", ssSettings->nMaxSongSearchResults, 50);
		LoadSetting(node, L"AlwaysSearchLibrary", ssSettings->bAlwaysSearchLibrary, true);
		LoadSetting(node, L"SSEnterAction", ssSettings->nSSEnterAction, 0);
		LoadSetting(node, L"SSShiftEnterAction", ssSettings->nSSShiftEnterAction, 1);
		LoadSetting(node, L"SSCtrlEnterAction", ssSettings->nSSCtrlEnterAction, 2);

		// Display settings
		LoadSetting(node, L"DisplayLayout", ssSettings->nDisplayLayout, 0);
		LoadSetting(node, L"NoDisplayWhenActive", ssSettings->bNoDisplayWhenActive, false);
		LoadSetting(node, L"OSDTopMost", ssSettings->bOSDTopMost, true);
		LoadSetting(node, L"OSDVert", ssSettings->nOSDVert, 100);
		LoadSetting(node, L"OSDHoriz", ssSettings->nOSDHoriz, 0);
		LoadSetting(node, L"OSDColorRef", ssSettings->lOSDColorRef, 255);
		LoadSetting(node, L"OSDFontPoint", ssSettings->nOSDFontPoint, 22);
		LoadSetting(node, L"OSDFontFace", ssSettings->wcFontFace, L"Segoe UI", 32);
		LoadSetting(node, L"OSDBkColorRef", ssSettings->lOSDBkColorRef, 4227327);
		LoadSetting(node, L"OSDFormat", ssSettings->wcOSDFormat, L"%track%\r\n%artist% - %album%\r\n%rating%", 512);
		LoadSetting(node, L"OSDAlpha", ssSettings->nOSDAlpha, 20);
		LoadSetting(node, L"OSDFadeEffects", ssSettings->bFadeEffects, true);
		LoadSetting(node, L"OSDForceWidth", ssSettings->bOSDForceWidth, false);
		LoadSetting(node, L"OSDMaxWidth", ssSettings->nOSDMaxWidth, -1);
		LoadSetting(node, L"OSDBorderColor", ssSettings->lOSDBorderColor, 0);
		LoadSetting(node, L"ArtBorderColor", ssSettings->lArtBorderColor, 0);
		LoadSetting(node, L"DropShadowColor", ssSettings->lDropShadowColor, 0);
		LoadSetting(node, L"OSDBorderSize", ssSettings->nOSDBorderSize, 3);
		LoadSetting(node, L"ArtBorderSize", ssSettings->nArtBorderSize, 1);
		LoadSetting(node, L"OSDFontStyle", ssSettings->nOSDFontStyle, 0);
		LoadSetting(node, L"OSDOutlineMode", ssSettings->bOSDOutlineMode, true);
		LoadSetting(node, L"OSDUseBorder", ssSettings->bOSDUseBorder, true);
		LoadSetting(node, L"OSDMonitor", ssSettings->nOSDMonitor, 1);
		LoadSetting(node, L"OSDAllMonitors", ssSettings->bOSDAllMonitors, false);
		LoadSetting(node, L"OSDTextJustify", ssSettings->nOSDTextJustify, 0);
		LoadSetting(node, L"OSDAlwaysUp", ssSettings->bOSDAlwaysUp, false);
		LoadSetting(node, L"OSDHideWhenPaused", ssSettings->bOSDHideWhenPaused, false);
		LoadSetting(node, L"OSDShowMissingTags", ssSettings->bOSDShowMissingTags, false);
		LoadSetting(node, L"UseDropShadow", ssSettings->bUseDropShadow, true);
		LoadSetting(node, L"DropShadowOffset", ssSettings->fDropShadowOffset, 1.5f);
		LoadSetting(node, L"NoDisplayForShortTracks", ssSettings->bNoDisplayShortTracks, false);
		LoadSetting(node, L"ShortTrackLength", ssSettings->nShortTrackLength, 60);

		// Ratings
		LoadSetting(node, L"RatingType", ssSettings->nRatingType, 2);
		wchar_t wcTemp[2] = { 0 };
		LoadSetting(node, L"RatingFullChar", wcTemp, L"*", 2);
		ssSettings->wcRatingFullChar = wcTemp[0];
		LoadSetting(node, L"RatingHalfChar", wcTemp, L"½", 2);
		ssSettings->wcRatingHalfChar = wcTemp[0];
		LoadSetting(node, L"RatingEmptyChar", wcTemp, L"", 2);
		ssSettings->wcRatingEmptyChar = wcTemp[0];

		GetCurrentDirectory(MAX_PATH, szPath);
		_tcscat_s(szPath, MAX_PATH, L"\\");
		_tcscat_s(szPath, MAX_PATH, L"full.png");
		LoadSetting(node, L"RatingFullPath", ssSettings->wcRatingFullPath, szPath, MAX_PATH);
		GetCurrentDirectory(MAX_PATH, szPath);
		_tcscat_s(szPath, MAX_PATH, L"\\");
		_tcscat_s(szPath, MAX_PATH, L"half.png");
		LoadSetting(node, L"RatingHalfPath", ssSettings->wcRatingHalfPath, szPath, MAX_PATH);
		GetCurrentDirectory(MAX_PATH, szPath);
		_tcscat_s(szPath, MAX_PATH, L"\\");
		_tcscat_s(szPath, MAX_PATH, L"empty.png");
		LoadSetting(node, L"RatingEmptyPath", ssSettings->wcRatingEmptyPath, szPath, MAX_PATH);

		LoadSetting(node, L"RatingShapeOutline", ssSettings->lRatingShapeOutline, 0);
		LoadSetting(node, L"RatingShapeFill", ssSettings->lRatingShapeFill, 0x00c0c0c0);
		LoadSetting(node, L"RatingShapeType", ssSettings->nRatingShapeType, 2);
		LoadSetting(node, L"RatingShapeStyle", ssSettings->nRatingShapeStyle, 0);

		// Art
		LoadSetting(node, L"OSDShowArt", ssSettings->bShowArtwork, true);
		LoadSetting(node, L"OSDArtConstSize", ssSettings->nArtConstSize, 100);
		LoadSetting(node, L"OSDArtMode", ssSettings->nArtMode, 3);
		LoadSetting(node, L"UseFolderArt", ssSettings->bFolderArt, true);
		LoadSetting(node, L"UseStaticArt", ssSettings->bStaticArt, false);
		LoadSetting(node, L"StaticArtPath", ssSettings->wcStaticArtPath, L"", MAX_PATH);
	}

	return;
}

HRESULT SaveHotkey(pugi::xml_node& node, const wchar_t* lpwcName, HotKeyItem hkiItem)
{
	pugi::xml_node child = node.child(lpwcName);

	if (!child)
	{
		node.append_child().set_name(lpwcName);
		child = node.child(lpwcName);
		child.append_attribute(L"mod");
		child.append_attribute(L"key");
	}

	child.attribute(L"mod").set_value(hkiItem.uiMods);
	child.attribute(L"key").set_value(hkiItem.uiKey);

	return S_OK;
}

HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, bool bValue)
{
	pugi::xml_node child = node.child(lpwcName);

	if (!child)
	{
		node.append_child().set_name(lpwcName);
		child = node.child(lpwcName);
		child.append_attribute(L"value");
	}

	child.attribute(L"value").set_value(bValue);

	return S_OK;
}

HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, __int64 nValue)
{
	pugi::xml_node child = node.child(lpwcName);
	wchar_t t[33] = { 0 };

	if (!child)
	{
		node.append_child().set_name(lpwcName);
		child = node.child(lpwcName);
		child.append_attribute(L"value");
	}

	_i64tow_s(nValue, t, 33, 10);
	child.attribute(L"value").set_value(t);

	return S_OK;
}

HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, int nValue)
{
	pugi::xml_node child = node.child(lpwcName);

	if (!child)
	{
		node.append_child().set_name(lpwcName);
		child = node.child(lpwcName);
		child.append_attribute(L"value");
	}

	child.attribute(L"value").set_value(nValue);

	return S_OK;
}

HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, float fValue)
{
	pugi::xml_node child = node.child(lpwcName);

	if (!child)
	{
		node.append_child().set_name(lpwcName);
		child = node.child(lpwcName);
		child.append_attribute(L"value");
	}

	child.attribute(L"value").set_value(fValue);

	return S_OK;
}

HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, long lValue)
{
	pugi::xml_node child = node.child(lpwcName);

	if (!child)
	{
		node.append_child().set_name(lpwcName);
		child = node.child(lpwcName);
		child.append_attribute(L"value");
	}

	child.attribute(L"value").set_value(lValue);

	return S_OK;
}

HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, const wchar_t* wcValue)
{
	pugi::xml_node child = node.child(lpwcName);

	if (!child)
	{
		node.append_child().set_name(lpwcName);
		child = node.child(lpwcName);
		child.append_attribute(L"value");
	}

	child.attribute(L"value").set_value(wcValue);

	return S_OK;
}

HRESULT SaveSetting(pugi::xml_node& node, const wchar_t* lpwcName, const char* cValue)
{
	size_t chars_conv = 0;
	wchar_t *wcTemp = NULL;

	mbstowcs_s(&chars_conv, NULL, 0, cValue, _TRUNCATE);

	wcTemp = new wchar_t[chars_conv];
	mbstowcs_s(&chars_conv, wcTemp, chars_conv, cValue, _TRUNCATE);

	SaveSetting(node, lpwcName, wcTemp);
	delete[] wcTemp;

	return S_OK;
}

void SaveSettings(vector<HotKeyItem> *vKeys, SettingsStruct *ssSettings)
{
	wchar_t szPath[MAX_PATH];
	if (!IsPortable())
	{
		if (SUCCEEDED(SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, L"iTunesControl", szPath)))
			PathAppend(szPath, L"config.xml");
	}
	else
	{
		GetModuleFileName(NULL, szPath, MAX_PATH);
		PathRemoveFileSpec(szPath);
		PathAppend((LPWSTR)szPath, L"config.xml");
	}

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(szPath);

	if (result.status == pugi::status_file_not_found)
	{
		if (!doc.load(WCONFSTR))
		{
			MessageBox(NULL, L"Fatal error while attempting to load settings!", L"Fatal Error", MB_OK | MB_ICONHAND);
			exit(-1);
		}
	}
	else if (result.status != pugi::status_ok)
	{
		MessageBox(NULL, L"Fatal error while attempting to load settings!", L"Fatal Error", MB_OK | MB_ICONHAND);
		exit(-1);
	}

	pugi::xml_node root = doc.child(L"itc");
	pugi::xml_node node = root.child(L"hotkeys");

	//Hotkeys
	if (vKeys)
	{
		vector<HotKeyItem>::iterator p = vKeys->begin();

		while (p != vKeys->end())
		{
			SaveHotkey(node, p->wcSettingsName, *p);

			++p;
		}
	}

	node = root.child(L"settings");

	//General Settings
	if (ssSettings)
	{
		SaveSetting(node, L"PromptQuitiTunes", ssSettings->bPromptQuitiTunes);
		SaveSetting(node, L"QuitiTunesOnExit", ssSettings->bQuitiTunesOnExit);
		SaveSetting(node, L"StartiTunesAtStart", ssSettings->bStartiTunesAtStart);
		SaveSetting(node, L"StartWithWindows", ssSettings->bStartWithWindows);
		SaveSetting(node, L"StartWithiTunes", ssSettings->bStartWithiTunes);
		SaveSetting(node, L"StartOnHotkeys", ssSettings->bStartOnHotkeys);
		SaveSetting(node, L"MinimizeiTunesAtStart", ssSettings->bMinimizeiTunesAtStart);
		SaveSetting(node, L"ShowCfgSaveWarning", ssSettings->bShowCfgSaveWarning);
		SaveSetting(node, L"ShowInTray", ssSettings->bShowInTray);
		SaveSetting(node, L"iTunesTitle", ssSettings->wciTunesTitle);
		SaveSetting(node, L"CSIFormat", ssSettings->wcCSIFormat);
		SaveSetting(node, L"QuickPlaylist", ssSettings->wcQuickPlaylist);

		SaveSetting(node, L"InfoShowTime", ssSettings->nPopupShowTime);
		SaveSetting(node, L"DisplayEndTime", ssSettings->nDisplayEndTime);
		SaveSetting(node, L"DisplayStartTime", ssSettings->nDisplayStartTime);
		SaveSetting(node, L"ShowBeginOffset", ssSettings->nShowBeginOffset);
		SaveSetting(node, L"ShowEndOffset", ssSettings->nShowEndOffset);
		SaveSetting(node, L"AllowBlankLines", ssSettings->bAllowBlankLines);
		SaveSetting(node, L"ShowDisplayAtEnd", ssSettings->bShowDisplayAtEnd);
		SaveSetting(node, L"ShowDisplayAtStart", ssSettings->bShowDisplayAtStart);
		SaveSetting(node, L"VolumeChangeInt", ssSettings->nVolumeChgInt);
		SaveSetting(node, L"VisualFeedback", ssSettings->bVisualFeedback);
		SaveSetting(node, L"QuitWithiTunes", ssSettings->bQuitWithiTunes);
		SaveSetting(node, L"TagField", ssSettings->nTagField);
		SaveSetting(node, L"JumpOneType", ssSettings->nJumpOneType);
		SaveSetting(node, L"JumpTwoType", ssSettings->nJumpTwoType);
		SaveSetting(node, L"JumpOneValue", ssSettings->nJumpOneValue);
		SaveSetting(node, L"JumpTwoValue", ssSettings->nJumpTwoValue);
		SaveSetting(node, L"RatingDelta", ssSettings->nRatingDelta);
		SaveSetting(node, L"PromptOnDelete", ssSettings->bPromptOnDelete);
		SaveSetting(node, L"DeleteFromDisk", ssSettings->bDeleteFromDisk);
		SaveSetting(node, L"DisableACOnFocus", ssSettings->bDisableACOnFocus);

		// Song Search
		SaveSetting(node, L"MaxSongSearchResults", ssSettings->nMaxSongSearchResults);
		SaveSetting(node, L"AlwaysSearchLibrary", ssSettings->bAlwaysSearchLibrary);
		SaveSetting(node, L"SSEnterAction", ssSettings->nSSEnterAction);
		SaveSetting(node, L"SSShiftEnterAction", ssSettings->nSSShiftEnterAction);
		SaveSetting(node, L"SSCtrlEnterAction", ssSettings->nSSCtrlEnterAction);

		// Display Settings
		SaveSetting(node, L"DisplayLayout", ssSettings->nDisplayLayout);
		SaveSetting(node, L"NoDisplayWhenActive", ssSettings->bNoDisplayWhenActive);
		SaveSetting(node, L"OSDTopMost", ssSettings->bOSDTopMost);
		SaveSetting(node, L"OSDVert", ssSettings->nOSDVert);
		SaveSetting(node, L"OSDHoriz", ssSettings->nOSDHoriz);
		SaveSetting(node, L"OSDColorRef", ssSettings->lOSDColorRef);
		SaveSetting(node, L"OSDFontPoint", ssSettings->nOSDFontPoint);
		SaveSetting(node, L"OSDFontFace", ssSettings->wcFontFace);
		SaveSetting(node, L"OSDBkColorRef", ssSettings->lOSDBkColorRef);
		SaveSetting(node, L"DropShadowColor", ssSettings->lDropShadowColor);
		SaveSetting(node, L"OSDFormat", ssSettings->wcOSDFormat);
		SaveSetting(node, L"OSDAlpha", ssSettings->nOSDAlpha);
		SaveSetting(node, L"OSDFadeEffects", ssSettings->bFadeEffects);
		SaveSetting(node, L"OSDForceWidth", ssSettings->bOSDForceWidth);
		SaveSetting(node, L"OSDMaxWidth", ssSettings->nOSDMaxWidth);
		SaveSetting(node, L"OSDBorderColor", ssSettings->lOSDBorderColor);
		SaveSetting(node, L"ArtBorderColor", ssSettings->lArtBorderColor);
		SaveSetting(node, L"OSDBorderSize", ssSettings->nOSDBorderSize);
		SaveSetting(node, L"ArtBorderSize", ssSettings->nArtBorderSize);
		SaveSetting(node, L"OSDFontStyle", ssSettings->nOSDFontStyle);
		SaveSetting(node, L"OSDOutlineMode", ssSettings->bOSDOutlineMode);
		SaveSetting(node, L"OSDUseBorder", ssSettings->bOSDUseBorder);
		SaveSetting(node, L"OSDMonitor", ssSettings->nOSDMonitor);
		SaveSetting(node, L"OSDAllMonitors", ssSettings->bOSDAllMonitors);
		SaveSetting(node, L"OSDTextJustify", ssSettings->nOSDTextJustify);
		SaveSetting(node, L"OSDAlwaysUp", ssSettings->bOSDAlwaysUp);
		SaveSetting(node, L"OSDHideWhenPaused", ssSettings->bOSDHideWhenPaused);
		SaveSetting(node, L"OSDShowMissingTags", ssSettings->bOSDShowMissingTags);
		SaveSetting(node, L"UseDropShadow", ssSettings->bUseDropShadow);
		SaveSetting(node, L"DropShadowOffset", ssSettings->fDropShadowOffset);
		SaveSetting(node, L"NoDisplayForShortTracks", ssSettings->bNoDisplayShortTracks);
		SaveSetting(node, L"ShortTrackLength", ssSettings->nShortTrackLength);
		SaveSetting(node, L"OnlyToggleRepeatOne", ssSettings->bOnlyToggleRepeatOne);

		// Ratings
		SaveSetting(node, L"RatingType", ssSettings->nRatingType);
		wchar_t wcTemp[2] = { 0 };
		wcTemp[0] = ssSettings->wcRatingFullChar;
		SaveSetting(node, L"RatingFullChar", wcTemp);
		wcTemp[0] = ssSettings->wcRatingHalfChar;
		SaveSetting(node, L"RatingHalfChar", wcTemp);
		wcTemp[0] = ssSettings->wcRatingEmptyChar;
		SaveSetting(node, L"RatingEmptyChar", wcTemp);
		SaveSetting(node, L"RatingFullPath", ssSettings->wcRatingFullPath);
		SaveSetting(node, L"RatingHalfPath", ssSettings->wcRatingHalfPath);
		SaveSetting(node, L"RatingEmptyPath", ssSettings->wcRatingEmptyPath);

		SaveSetting(node, L"RatingShapeOutline", ssSettings->lRatingShapeOutline);
		SaveSetting(node, L"RatingShapeFill", ssSettings->lRatingShapeFill);
		SaveSetting(node, L"RatingShapeType", ssSettings->nRatingShapeType);
		SaveSetting(node, L"RatingShapeStyle", ssSettings->nRatingShapeStyle);

		// Art
		SaveSetting(node, L"OSDShowArt", ssSettings->bShowArtwork);
		SaveSetting(node, L"OSDArtConstSize", ssSettings->nArtConstSize);
		SaveSetting(node, L"OSDArtMode", ssSettings->nArtMode);
		SaveSetting(node, L"UseFolderArt", ssSettings->bFolderArt);
		SaveSetting(node, L"UseStaticArt", ssSettings->bStaticArt);
		SaveSetting(node, L"StaticArtPath", ssSettings->wcStaticArtPath);
	}

	doc.save_file(szPath);

	return;
}
