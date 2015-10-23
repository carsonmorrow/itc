#ifndef __COM_H__INCLUDED
#define __COM_H__INCLUDED

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <comutil.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <string>
#include <sstream>
#include <cctype>
#include <vector>
using namespace std;

#include "globaldata.h"
#include "settings.h"
#include "resource.h"
#include "InputEngine.h"
#include "GraphicsInterface.h"

#include "TEventHandler.h"
using namespace TEventHandlerNamespace;

#pragma warning(disable : 4251 4996)

template <class T> std::wstring to_string(const T & t) { std::wostringstream woss; woss << t; return woss.str(); }

struct sTrackInfo
{
	wstring wstrArtist;
	wstring wstrTitle;
	wstring wstrAlbum;
	wstring wstrTags;
	wstring wstrComment;
	wstring wstrRating;
	wstring wstrGenre;
	long nTrackLength;
	long nBitRate;
	wstring wstrTotalTracks;
	wstring wstrComposer;
	long lRating;
	wstring wstrTrackNum;
	long lNormalize;
	wstring wstrYear;
};

class CITunesCOM;
struct IiTunes abstract;
struct _IiTunesEvents;

typedef TEventHandler<CITunesCOM, IiTunes, _IiTunesEvents> IiTunesEventHandler;

class CITunesCOM {
public:
	//Constructor/Destructor
	CITunesCOM();
	~CITunesCOM();

	//Utility functions
	void Connect(bool bStartIt);
	void Disconnect();
	void Quit();
	bool IsConnected();
	void Stabilize();

	bool IsActiveWindow() { return ( GetForegroundWindow() == m_hWndITunes ); };

	//Action functions
	void PlayPause();
	bool IsPlaying();
	void Pause();
	void Stop();
	void Delete();
	void NextTrack();
	void PreviousTrack();
	void VolumeDown(int nInterval);
	void VolumeUp(int nInterval);
	void VolumeMute();
	unsigned int GetVolumeLevel();
	int AdvancedSeek(int nType, int nDir);
	void ToggleShuffle();
	void CycleRepeatMode();
	void FastForward(long lSeconds);
	void Rewind(long lSeconds);
	void ShowHide();
	void UpdateIPod();
	void QuickPlaylist( wstring wstrPlaylist );
	void PlaylistSearch( vector<wstring> *vPlaylists );
	void SongSearch( wstring wstrSearch );
	void PlaySSResult( long lTrackID, long lTrackDBID );
	void Jump( int nType, int nValue );
	void StopAfterCurrent( );
	void ToggleTrackEnable( );
	long GetTimeLeftInTrack( );
	long GetTimeInTrack( );

	void PlayInCurrentPlaylist( long lTrackID, long lTrackDBID );
	void PlayIniTunesDJ();
	void PlayIniTunesDJ( long lTrackID, long lTrackDBID );

	void SetTrackRating(long lRating);
	inline long GetTrackRating() { return stiCurrent.lRating; }

	inline long GetTrackLength() { return stiCurrent.nTrackLength; }

	void SetTrackNormalize(long lNormalize);
	inline long GetTrackNormalize() { return stiCurrent.lNormalize; }

	void SetWindowCaption(bool bReset = false);

	inline HWND GetiTunesHWND() { return m_hWndITunes; }

	void SetSleepTimer(int nMinutes);
	void ClearSleepTimer();

	//void TestFunction( );

	void FormatTitle(wstring *wstrFormat);
	void ShowCurrentInfo( int display_time=m_ssSettings.nPopupShowTime, bool bUpdate=false );
	void DisplayFormattedText(wstring *wstrText);

	bool FindFolderArt();
	bool DumpArtwork();
	void KillArtworkTemp();
	void ReloadArtwork();

	void SaveTags();

	wchar_t m_wcArtTemp[MAX_PATH + 1];
	bool m_bArtIsTemp;
	bool m_bDisplayArtwork;
	bool m_bSleepOn;
	vector<wstring> _wstrTags;

private:
	IiTunes* iITunes;
	HWND m_hWndITunes;
	bool m_bGood;

	long m_lSourceID;
	long m_lPlaylistID;

	bool _bStopAfterCurrent;

	bool _bSuppressNotifications;

	sTrackInfo stiCurrent;

	void GetTrackInfo();
	void ReplaceConstants( wstring & wstrLine, bool bAllowImageRatings = true );
	void Replace(wstring & wstrText, wstring wstrFind, wstring wstrReplace);
	void CommentToTags(wstring & wstrText);
	void MakeRatingsText( long lRating, wstring *wstrRating );

	bool IsStable() { return (m_bGood && iITunes ); }

protected:
	IiTunesEventHandler* m_pIiTunesEventHandler;

	HRESULT CITunesCOM::OnEventFiringObjectInvoke
	(
	  IiTunesEventHandler* pEventHandler,
	  DISPID dispidMember, 
	  REFIID riid,
	  LCID lcid, 
	  WORD wFlags, 
	  DISPPARAMS* pdispparams, 
	  VARIANT* pvarResult,
	  EXCEPINFO* pexcepinfo, 
	  UINT* puArgErr
	);
};

const int WM_SETSTATE = WM_APP + 3;

#endif