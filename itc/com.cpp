#define WINVER 0x0502
#define _WIN32_WINNT 0x0502
#define _WIN32_IE 0x0600

#include "itunescominterface.h"
#include "com.h"
#include <comdef.h>
#include "memcheck.h"

_COM_SMARTPTR_TYPEDEF(IITTrack, __uuidof(IITTrack));
_COM_SMARTPTR_TYPEDEF(IITPlaylist, __uuidof(IITPlaylist));
_COM_SMARTPTR_TYPEDEF(IITObject, __uuidof(IITObject));
_COM_SMARTPTR_TYPEDEF(IITSource, __uuidof(IITSource));
_COM_SMARTPTR_TYPEDEF(IITArtwork, __uuidof(IITArtwork));
_COM_SMARTPTR_TYPEDEF(IITLibraryPlaylist, __uuidof(IITLibraryPlaylist));
_COM_SMARTPTR_TYPEDEF(IITUserPlaylist, __uuidof(IITUserPlaylist));
_COM_SMARTPTR_TYPEDEF(IITTrackCollection, __uuidof(IITTrackCollection));
_COM_SMARTPTR_TYPEDEF(IITSourceCollection, __uuidof(IITSourceCollection));
_COM_SMARTPTR_TYPEDEF(IITPlaylistCollection, __uuidof(IITPlaylistCollection));
_COM_SMARTPTR_TYPEDEF(IITArtworkCollection, __uuidof(IITArtworkCollection));
_COM_SMARTPTR_TYPEDEF(IITFileOrCDTrack, __uuidof(IITFileOrCDTrack));

CITunesCOM::CITunesCOM()
{
	iITunes = NULL;
	m_bGood = false;
	m_hWndITunes = NULL;
	m_bSleepOn = false;
	m_pIiTunesEventHandler = NULL;
	_bStopAfterCurrent = false;
	_bSuppressNotifications = false;

	memset(m_wcArtTemp, 0, MAX_PATH);

	stiCurrent.lRating = 0;
	stiCurrent.nBitRate = 0;
	stiCurrent.nTrackLength = 0;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
}

void CITunesCOM::DisplayFormattedText(wstring *wstrText)
{
	if (wstrText != NULL && _ginterface != NULL)
	{
		ReplaceConstants(*wstrText);
		_ginterface->DisplayStatus(*wstrText);
	}
}

void CITunesCOM::StopAfterCurrent()
{
	if (_ginterface == NULL)
		return;

	_bStopAfterCurrent = !_bStopAfterCurrent;

	if (_bStopAfterCurrent)
	{
		_ginterface->DisplayStatus(L"Stop After Current Track Enabled");
	}
	else
	{
		_ginterface->DisplayStatus(L"Stop After Current Track Disabled");
	}

	return;
}

void CITunesCOM::SongSearch(wstring wstrSearch)
{
	HRESULT hRes = NULL;
	IITPlaylistPtr iPlaylist;
	IITLibraryPlaylistPtr iLibraryPlaylist;
	IITTrackCollectionPtr iTrackCollection;
	IUnknown *punk = NULL;
	IEnumVARIANT *pev = NULL;
	VARIANT v;
	IITTrackPtr iTrack;
	wstring wstrTemp;
	wchar_t wcDisplay[255 * 2 + 4]; // artist+track+" - "+\0
	long lTrackDBID, lTrackID, lItem;
	_bstr_t bstrTrack, bstrArtist;
	_bstr_t bstrSearchString = wstrSearch.c_str();
	int nResults = 0;

	if (!IsStable())
		return;

	try
	{
		if (m_ssSettings.bAlwaysSearchLibrary)
		{
			hRes = iITunes->get_LibraryPlaylist(&iLibraryPlaylist);
			if (S_OK != hRes)
				return;

			iPlaylist = IITPlaylistPtr(iLibraryPlaylist);
			if (iPlaylist == NULL)
				return;
		}
		else
			hRes = iITunes->get_CurrentPlaylist(&iPlaylist);
		if (S_OK != hRes)
		{
			hRes = iITunes->get_LibraryPlaylist(&iLibraryPlaylist);
			if (S_OK != hRes)
				return;

			iPlaylist = IITPlaylistPtr(iLibraryPlaylist);
			if (iPlaylist == NULL)
				return;
		}

		hRes = iPlaylist->Search(bstrSearchString, ITPlaylistSearchFieldAll, &iTrackCollection);
		if (S_OK != hRes || NULL == iTrackCollection)
		{
			swprintf_s(wcDisplay, 255 * 2 + 4, L"Song Search [%d results]", nResults);
			SetWindowText(m_hWndSongSearch, wcDisplay);
			return;
		}

		hRes = iTrackCollection->get__NewEnum(&punk);
		if (S_OK != hRes)
			return;

		hRes = punk->QueryInterface(__uuidof(IEnumVARIANT), (void **)&pev);
		punk->Release();
		if (S_OK != hRes)
			return;

		iPlaylist->get_PlaylistID(&m_lPlaylistID);
		iPlaylist->get_SourceID(&m_lSourceID);

		VariantInit(&v);
		while (S_OK == pev->Next(1, &v, NULL) && nResults < m_ssSettings.nMaxSongSearchResults)
		{
			if (VT_DISPATCH == V_VT(&v))
			{
				V_DISPATCH(&v)->QueryInterface(__uuidof(IITTrack), (void **)&iTrack);
				if (iTrack)
				{
					if (WAIT_OBJECT_0 == WaitForSingleObject(g_hSS_Stop, 0))
					{
						VariantClear(&v);
						swprintf_s(wcDisplay, 255 * 2 + 4, L"Song Search [%d results]", nResults);
						SetWindowText(m_hWndSongSearch, wcDisplay);
						pev->Release();
						return;
					}

					iTrack->get_Name(bstrTrack.GetAddress());
					iTrack->get_Artist(bstrArtist.GetAddress());
					iTrack->get_TrackID(&lTrackID);
					iTrack->get_TrackDatabaseID(&lTrackDBID);

					swprintf_s(wcDisplay, 255 * 2 + 4, L"%s%s%s", (!bstrArtist ? L"" : bstrArtist), (!bstrArtist ? L"" : L" - "),
						(!bstrTrack ? L"" : bstrTrack));

					lItem = SendMessage(m_hWndSongSearchListBox, LB_ADDSTRING, 0, (LPARAM)wcDisplay);
					if (LB_ERR != lItem)
					{
						SendMessage(m_hWndSongSearchListBox, LB_SETITEMDATA, lItem, MAKELONG(lTrackID, lTrackDBID));
						nResults++;
						swprintf_s(wcDisplay, 255 * 2 + 4, L"Song Search [%d results] Searching...", nResults);
						SetWindowText(m_hWndSongSearch, wcDisplay);
					}
				}
			}
			VariantClear(&v);
		}
		pev->Release();
	}
	catch (...)
	{
	}

	swprintf_s(wcDisplay, 255 * 2 + 4, L"Song Search [%d results]", nResults);
	SetWindowText(m_hWndSongSearch, wcDisplay);
	SendMessage(m_hWndSongSearchListBox, LB_SETCURSEL, 0, 0);

	return;
}

void CITunesCOM::PlaySSResult(long lTrackID, long lTrackDBID)
{
	HRESULT hRes;
	IITObjectPtr iObject;
	IITTrackPtr iTrack;

	if (!IsStable())
		return;

	try
	{
		hRes = iITunes->GetITObjectByID(m_lSourceID, m_lPlaylistID, lTrackID, lTrackDBID, &iObject);

		if (S_OK != hRes)
			return;

		iTrack = IITTrackPtr(iObject);
		if (iTrack == NULL)
			return;

		iTrack->Play();
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::PlaylistSearch(vector<wstring> *vPlaylists)
{
	HRESULT hRes = NULL;
	IITSourceCollectionPtr iSourceCollection;
	IITPlaylistCollectionPtr iPlaylistCollection;
	IITSourcePtr iSource;
	IITPlaylistPtr iPlaylist;
	long lCount = 0;
	long lCountPlaylists = 0;
	_bstr_t bstrPlaylistName;
	wstring wstrTemp(L"");

	if (!IsStable() || vPlaylists == NULL)
		return;

	try
	{
		hRes = iITunes->get_Sources(&iSourceCollection);
		if (S_OK != hRes)
			return;

		hRes = iSourceCollection->get_Count(&lCount);
		if (S_OK != hRes)
			return;

		for (; lCount > 0; lCount--)
		{
			hRes = iSourceCollection->get_Item(lCount, &iSource);
			if (S_OK != hRes)
				return;

			hRes = iSource->get_Playlists(&iPlaylistCollection);
			if (S_OK != hRes)
				return;

			hRes = iPlaylistCollection->get_Count(&lCountPlaylists);
			if (S_OK != hRes)
				return;

			for (; lCountPlaylists > 0; lCountPlaylists--)
			{
				hRes = iPlaylistCollection->get_Item(lCountPlaylists, &iPlaylist);
				if (S_OK != hRes)
					return;

				hRes = iPlaylist->get_Name(bstrPlaylistName.GetAddress());
				if (S_OK != hRes)
					return;

				wstrTemp = bstrPlaylistName;
				vPlaylists->push_back(wstrTemp);
			}
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::QuickPlaylist(wstring wstrPlaylist)
{
	IITSourcePtr iSource;
	IITPlaylistCollectionPtr iPlaylistCollection;
	IITPlaylistPtr iPlaylist;
	_bstr_t bstrPlaylist;

	if (!IsStable())
		return;


	HRESULT hRes = iITunes->get_LibrarySource(&iSource);
	if (S_OK != hRes)
		return;

	if (S_OK != iSource->get_Playlists(&iPlaylistCollection))
		return;

	bstrPlaylist = wstrPlaylist.c_str();
	if (!bstrPlaylist)
		return;

	if (S_OK == iPlaylistCollection->get_ItemByName(bstrPlaylist, &iPlaylist))
		iPlaylist->PlayFirstTrack();

	return;
}

void CITunesCOM::PlayInCurrentPlaylist(long lTrackID, long lTrackDBID)
{
	IITPlaylistPtr iPlaylist;
	IITUserPlaylistPtr iUserPlaylist;
	IITTrackPtr iTrackToAdd;
	IITTrackPtr iTrackAdded;
	IDispatch *pTrack = NULL;
	IITObjectPtr iObject;

	if (!IsStable())
		return;

	if (S_OK != iITunes->get_CurrentPlaylist(&iPlaylist))
		return;

	iUserPlaylist = IITUserPlaylistPtr(iPlaylist);
	if (NULL == iUserPlaylist)
		return;

	if (S_OK != iITunes->GetITObjectByID(m_lSourceID, m_lPlaylistID, lTrackID, lTrackDBID, &iObject))
		return;

	iTrackToAdd = IITTrackPtr(iObject);
	if (iTrackToAdd == NULL)
		return;

	iTrackToAdd->QueryInterface(IID_IDispatch, (void**)&pTrack);

	_variant_t vt(pTrack);
	if (S_OK == iUserPlaylist->AddTrack(&vt.GetVARIANT(), &iTrackAdded))
		iTrackAdded->Play();

	pTrack->Release();

	return;
}

void CITunesCOM::PlayIniTunesDJ()
{
	IITSourcePtr iSource;
	IITPlaylistCollectionPtr iPlaylistCollection;
	IITPlaylistPtr iPlaylist;
	IITUserPlaylistPtr iUserPlaylist;
	IITTrackPtr iTrackToAdd;
	IITTrackPtr iTrackAdded;
	_bstr_t bstrPlaylist;

	if (!IsStable())
		return;

	if (S_OK != iITunes->get_LibrarySource(&iSource))
		return;

	if (S_OK != iSource->get_Playlists(&iPlaylistCollection))
		return;

	bstrPlaylist = L"iTunes DJ";
	if (!bstrPlaylist)
		return;

	if (S_OK != iPlaylistCollection->get_ItemByName(bstrPlaylist, &iPlaylist))
	{
		// Try old party shuffle name
		bstrPlaylist = L"Party Shuffle";
		if (!bstrPlaylist)
			return;
		if (S_OK != iPlaylistCollection->get_ItemByName(bstrPlaylist, &iPlaylist))
			return;
	}

	iUserPlaylist = IITUserPlaylistPtr(iPlaylist);
	if (NULL == iUserPlaylist)
		return;

	if (S_OK != iITunes->get_CurrentTrack(&iTrackToAdd))
		return;

	IDispatch *pTrack = NULL;
	iTrackToAdd->QueryInterface(IID_IDispatch, (void**)&pTrack);

	_variant_t vt(pTrack);
	if (S_OK == iUserPlaylist->AddTrack(&vt.GetVARIANT(), &iTrackAdded))
		iTrackAdded->Play();

	pTrack->Release();

	return;
}

void CITunesCOM::PlayIniTunesDJ(long lTrackID, long lTrackDBID)
{
	IITSourcePtr iSource;
	IITPlaylistCollectionPtr iPlaylistCollection;
	IITPlaylistPtr iPlaylist;
	IITUserPlaylistPtr iUserPlaylist;
	IITTrackPtr iTrackToAdd;
	IITTrackPtr iTrackAdded;
	IITTrackPtr iTrackAdded2;
	_bstr_t bstrPlaylist;

	if (!IsStable())
		return;

	if (S_OK != iITunes->get_LibrarySource(&iSource))
		return;

	if (S_OK != iSource->get_Playlists(&iPlaylistCollection))
		return;

	bstrPlaylist = L"iTunes DJ";
	if (!bstrPlaylist)
		return;

	if (S_OK != iPlaylistCollection->get_ItemByName(bstrPlaylist, &iPlaylist))
	{
		// Try old party shuffle name
		bstrPlaylist = L"Party Shuffle";
		if (!bstrPlaylist)
			return;
		if (S_OK != iPlaylistCollection->get_ItemByName(bstrPlaylist, &iPlaylist))
			return;
	}

	iUserPlaylist = IITUserPlaylistPtr(iPlaylist);
	if (NULL == iUserPlaylist)
		return;

	IDispatch *pTrack = NULL;

	// ********
	// Add target track to end of playlist
	IITObjectPtr iObject;
	IITTrackPtr iTrack;
	if (S_OK != iITunes->GetITObjectByID(m_lSourceID, m_lPlaylistID, lTrackID, lTrackDBID, &iObject))
		return;

	iTrack = IITTrackPtr(iObject);
	if (iTrack == NULL)
		return;

	pTrack = NULL;
	iTrack->QueryInterface(IID_IDispatch, (void**)&pTrack);

	_variant_t vt2(pTrack);
	iUserPlaylist->AddTrack(&vt2.GetVARIANT(), &iTrackAdded2);

	pTrack->Release();

	return;
}

CITunesCOM::~CITunesCOM()
{
	if (m_pIiTunesEventHandler)
	{
		m_pIiTunesEventHandler->ShutdownConnectionPoint();
		m_pIiTunesEventHandler->Release();
		m_pIiTunesEventHandler = NULL;
	}

	if (iITunes)
		iITunes->Release();

	KillArtworkTemp();

	::CoUninitialize();
}

void CITunesCOM::Connect(bool bStartIt)
{
	HRESULT hRes = 0;
	DWORD dwSize = MAX_PATH;
	TCHAR tITunesPath[MAX_PATH];

	//find iTunes window
	m_hWndITunes = FindWindow(L"iTunes", L"iTunes");

	if (m_hWndITunes && NULL == GetWindow(m_hWndITunes, GW_CHILD))
		m_hWndITunes = FindWindowEx(NULL, m_hWndITunes, L"iTunes", L"iTunes");

	if (m_hWndITunes == NULL)
		m_hWndITunes = FindWindow(L"ITWindow", L"iTunes");

	// Couldn't find the window, and we're supposed to start it
	if (m_hWndITunes == NULL && bStartIt)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		DWORD dwRet = 0;
		HKEY hkRoot;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		if (m_ssSettings.bMinimizeiTunesAtStart)
		{
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_MINIMIZE;
		}
		ZeroMemory(&pi, sizeof(pi));

		dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\iTunes.exe", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ,
			NULL, &hkRoot, NULL);

		if (dwRet == ERROR_SUCCESS)
		{
			dwRet = RegQueryValueEx(hkRoot,
				NULL,
				NULL,
				NULL,
				(LPBYTE)&tITunesPath,
				&dwSize);

			RegCloseKey(hkRoot);

			if (dwRet == ERROR_SUCCESS)
			{
				// If iTunes COM isn't registered, try to take care of that first
				// Note: This won't work if iTunes is already running, so fails for start-with-itunes
				LPOLESTR lpProgID = NULL;
				HRESULT hr = ProgIDFromCLSID(CLSID_iTunesApp, &lpProgID);
				if (S_OK == hr)
					CoTaskMemFree((LPVOID)lpProgID);

				if (hr == REGDB_E_CLASSNOTREG)
				{
					wchar_t wcCommandLine[MAX_PATH * 2] = { 0 };

					swprintf_s(wcCommandLine, L"%s /regserver", tITunesPath);

					CreateProcess(tITunesPath, wcCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

					if (WAIT_OBJECT_0 == WaitForSingleObject(pi.hProcess, 3000))
						hRes = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iITunes);

					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}

				CreateProcess(tITunesPath, tITunesPath, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

				WaitForInputIdle(pi.hProcess, 5000);

				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
	}

	// If we found the window, or we're supposed to start it
	if (m_hWndITunes != NULL || bStartIt)
	{
		try
		{
			hRes = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iITunes);

			if (S_OK != hRes)
			{
				MessageBox(NULL, L"iTunesControl has encountered an error while connecting to iTunes. " \
					L"If iTunes is not installed, it must be installed prior to running iTunesControl. " \
					L"If iTunes is installed, this likely means that your installation of iTunes is corrupted. " \
					L"The recommended resolution is to uninstall iTunes, restart your computer, and reinstall iTunes.",
					L"Error Communicating with iTunes!", MB_OK);
				throw (1); // if we couldn't create instance, just leave
			}

			m_bGood = true;

			m_pIiTunesEventHandler = new IiTunesEventHandler(*this, iITunes, &CITunesCOM::OnEventFiringObjectInvoke);

			this->GetTrackInfo();
			this->CommentToTags(stiCurrent.wstrTags);
			this->SetWindowCaption();
			this->ReloadArtwork();
			this->ShowCurrentInfo();

			// Theoretically, this will stop iTunes from processing WM_APPCOMMAND...
			VARIANT_BOOL vb = VARIANT_FALSE;
			iITunes->put_AppCommandMessageProcessingEnabled(vb);
		}
		catch (...)
		{
			_RPTFW0(_CRT_WARN, L"Caught exception while creating iTunes COM instance!");
			m_bGood = false;
			iITunes = NULL;
			m_pIiTunesEventHandler = NULL;
			m_hWndITunes = NULL;
		}
	}

	// If we created the COM instance, but haven't gotten the window yet, find it
	if (m_hWndITunes == NULL && m_bGood)
	{
		//find iTunes window
		m_hWndITunes = FindWindow(L"iTunes", L"iTunes");

		if (m_hWndITunes && NULL == GetWindow(m_hWndITunes, GW_CHILD))
			m_hWndITunes = FindWindowEx(NULL, m_hWndITunes, L"iTunes", L"iTunes");

		if (m_hWndITunes == NULL)
			m_hWndITunes = FindWindow(L"ITWindow", L"iTunes");
	}

	if (m_ssSettings.bMinimizeiTunesAtStart && m_hWndITunes)
		ShowWindow(m_hWndITunes, SW_MINIMIZE);

	return;
}

void CITunesCOM::Disconnect()
{
	try
	{
		if (m_bGood)
		{
			if (m_pIiTunesEventHandler)
			{
				m_pIiTunesEventHandler->ShutdownConnectionPoint();
				m_pIiTunesEventHandler->Release();
				m_pIiTunesEventHandler = NULL;
			}

			if (iITunes)
				iITunes->Release();

			m_bGood = false;
			m_hWndITunes = NULL;
			iITunes = NULL;
			_bStopAfterCurrent = false;
		}
	}
	catch (...)
	{
		// Something went very, very wrong
		// Clear handles to be safe (possible leaking them)
		m_bGood = false;
		m_hWndITunes = NULL;
		iITunes = NULL;
		m_pIiTunesEventHandler = NULL;
	}

	return;
}

void CITunesCOM::Quit()
{
	if (m_bGood)
	{
		if (m_pIiTunesEventHandler)
		{
			m_pIiTunesEventHandler->ShutdownConnectionPoint();
			m_pIiTunesEventHandler->Release();
			m_pIiTunesEventHandler = NULL;
		}

		if (iITunes)
		{
			iITunes->Quit();
			iITunes->Release();
		}

		iITunes = NULL;
		m_bGood = false;
		m_hWndITunes = NULL;
	}

	return;
}

HRESULT CITunesCOM::OnEventFiringObjectInvoke(IiTunesEventHandler*, DISPID dispidMember, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*)
{
	if (!IsStable())
		return S_OK;

	if (ITEventAboutToPromptUserToQuit == dispidMember)
	{
		HWND hwnd = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");

		PostMessage(hwnd, WM_SETSTATE, 0, 0);
		_ginterface->DisplayHide();
		KillTimer(hwnd, 15);

		this->Disconnect();
	}
	else if ((ITEventPlayerPlay == dispidMember || ITEventPlayerPlayingTrackChanged == dispidMember) && !_bSuppressNotifications)
	{
		if (_bStopAfterCurrent)
		{
			iITunes->Stop();

			_bStopAfterCurrent = false;

			return S_OK;
		}

		this->GetTrackInfo();
		this->CommentToTags(stiCurrent.wstrTags);
		this->SetWindowCaption();
		this->ReloadArtwork();
		if ((_ginterface->IsDisplayVisible() || m_ssSettings.bOSDAlwaysUp || (m_ssSettings.bShowDisplayAtStart && m_ssSettings.nShowBeginOffset == 0)) &&
			!(m_ssSettings.bNoDisplayShortTracks && stiCurrent.nTrackLength < m_ssSettings.nShortTrackLength))
		{
			this->ShowCurrentInfo();
		}
		else
		{
			_ginterface->DisplayHide();
		}
	}
	else if (ITEventPlayerStop == dispidMember)
	{
		HRESULT hRes;
		IITTrackPtr iTrack;

		try
		{
			hRes = iITunes->get_CurrentTrack(&iTrack);
			if (S_OK != hRes || NULL == iTrack)
			{
				this->SetWindowCaption(true);
				this->KillArtworkTemp();
				_ginterface->DisplayHide();
			}
		}
		catch (...)
		{
		}
	}

	return S_OK;
}

void CITunesCOM::ReloadArtwork()
{
	try
	{
		this->KillArtworkTemp();
		if (m_ssSettings.bStaticArt)
		{
			wcscpy_s(m_wcArtTemp, MAX_PATH + 1, m_ssSettings.wcStaticArtPath);
			m_bArtIsTemp = false;
		}
		else if (false == this->DumpArtwork())
		{
			this->FindFolderArt();
		}
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in ReloadArtwork().");
	}
}

bool CITunesCOM::IsPlaying()
{
	try
	{
		if (IsStable())
		{
			ITPlayerState iTPlayerState;

			if (iITunes->get_PlayerState(&iTPlayerState) == S_OK)
			{
				if (iTPlayerState != ITPlayerStatePlaying)
					return false;
				else
					return true;
			}
			else
				return false;
		}
		else
			return false;
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in IsPlaying().");
		return false;
	}
}

void CITunesCOM::PlayPause()
{
	try
	{
		if (IsStable())
		{
			ITPlayerState iTPlayerState;

			iITunes->PlayPause();

			if (iITunes->get_PlayerState(&iTPlayerState) == S_OK)
			{
				if (iTPlayerState != ITPlayerStatePlaying)
				{
					wchar_t wcTemp[50] = { 0 };

					_stprintf_s(wcTemp, 50, _T("%s\n%s"), L"iTunes State", L"Paused");

					if (m_ssSettings.bOSDAlwaysUp && m_ssSettings.bOSDHideWhenPaused)
						_ginterface->DisplayHide();
					else
						_ginterface->DisplayStatus(wcTemp);
				}
			}
		}
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in PlayPause().");
	}

	return;
}

void CITunesCOM::Pause()
{
	try
	{
		if (IsStable())
		{
			iITunes->Pause();

			wchar_t wcTemp[50] = { 0 };

			_stprintf_s(wcTemp, 50, _T("%s\n%s"), L"iTunes State", L"Paused");

			_ginterface->DisplayStatus(wcTemp);
		}
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in PlayPause().");
	}

	return;
}

void CITunesCOM::Stop()
{
	try
	{
		if (IsStable())
		{
			iITunes->Stop();

			wchar_t wcTemp[50] = { 0 };

			_stprintf_s(wcTemp, 50, _T("%s\n%s"), L"iTunes State", L"Stopped");

			_ginterface->DisplayStatus(wcTemp);
		}
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in Stop().");
	}

	return;
}

void CITunesCOM::FastForward(long lSeconds)
{
	try
	{
		if (IsStable())
		{
			long lPos = 0;
			HRESULT hRes = 0;

			hRes = iITunes->get_PlayerPosition(&lPos);

			if (hRes == S_OK)
			{
				iITunes->put_PlayerPosition(lPos + lSeconds);
			}
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::Rewind(long lSeconds)
{
	try
	{
		if (IsStable())
		{
			long lPos = 0;
			HRESULT hRes = 0;

			hRes = iITunes->get_PlayerPosition(&lPos);

			if (hRes == S_OK)
			{
				iITunes->put_PlayerPosition(lPos - lSeconds);
			}
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::SetTrackRating(long lRating)
{
	HRESULT hRes = 0;
	IITTrackPtr iTrack = NULL;

	try
	{
		if (IsStable())
		{
			hRes = iITunes->get_CurrentTrack(&iTrack);

			if (hRes == S_OK && iTrack != NULL)
			{
				// Changing track metadata _should_ give us a notification, which would update
				// the display. For some reason, this isn't happening for some users. So...
				iTrack->put_Rating(lRating);
				// we'll fake an update, just in case.
				this->GetTrackInfo();
				this->SetWindowCaption();
				this->ShowCurrentInfo();
			}
		}
	}
	catch (...)
	{
	}
}

void CITunesCOM::NextTrack()
{
	try
	{
		if (IsStable())
		{
			iITunes->NextTrack();
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::PreviousTrack()
{
	try
	{
		if (IsStable())
		{
			iITunes->PreviousTrack();
		}
	}
	catch (...)
	{
	}

	return;
}

unsigned int CITunesCOM::GetVolumeLevel()
{
	try
	{
		if (IsStable())
		{
			HRESULT hRes;
			long lVolume = 0;

			hRes = iITunes->get_SoundVolume(&lVolume);

			return (unsigned int)lVolume;
		}
	}
	catch (...)
	{
	}

	return 0;
}

void CITunesCOM::VolumeDown(int nInterval)
{
	try
	{
		if (IsStable())
		{
			HRESULT hRes;
			long lVolume = 0;
			VARIANT_BOOL bMute = 0;

			// Unmute if muted
			hRes = iITunes->get_Mute(&bMute);
			if (S_OK == hRes && VARIANT_TRUE == bMute)
			{
				bMute = VARIANT_FALSE;
				iITunes->put_Mute(bMute);
			}

			hRes = iITunes->get_SoundVolume(&lVolume);

			lVolume -= nInterval;

			hRes = iITunes->put_SoundVolume(lVolume);

			wstring wstrVolumeLevel = L"%volume%";

			_ginterface->DisplayStatus(wstrVolumeLevel);
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::VolumeUp(int nInterval)
{
	try
	{
		if (IsStable())
		{
			HRESULT hRes;
			long lVolume = 0;
			VARIANT_BOOL bMute = 0;

			// Unmute if muted
			hRes = iITunes->get_Mute(&bMute);
			if (S_OK == hRes && VARIANT_TRUE == bMute)
			{
				bMute = VARIANT_FALSE;
				iITunes->put_Mute(bMute);
			}

			hRes = iITunes->get_SoundVolume(&lVolume);

			lVolume += nInterval;

			hRes = iITunes->put_SoundVolume(lVolume);

			wstring wstrVolumeLevel = L"%volume%";

			_ginterface->DisplayStatus(wstrVolumeLevel);
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::VolumeMute()
{
	try
	{
		if (IsStable())
		{
			HRESULT hRes;
			VARIANT_BOOL bMute = 0;

			hRes = iITunes->get_Mute(&bMute);
			if (S_OK != hRes)
				return;

			if (VARIANT_FALSE == bMute)
			{
				bMute = VARIANT_TRUE;
				_ginterface->DisplayStatus(L"iTunes Mute On");
			}
			else
			{
				bMute = VARIANT_FALSE;
				_ginterface->DisplayStatus(L"iTunes Mute Off");
			}

			iITunes->put_Mute(bMute);
		}
	}
	catch (...)
	{
	}

	return;
}

bool CITunesCOM::IsConnected()
{
	if (IsStable() && m_hWndITunes != NULL)
		return true;
	else
		return false;
}

void CITunesCOM::SetWindowCaption(bool bReset)
{
	HRESULT hRes;
	IITTrackPtr iTrack;

	if (!IsStable())
		return;

	try
	{
		if (bReset == false)
		{
			hRes = iITunes->get_CurrentTrack(&iTrack);
			if (S_OK != hRes || NULL == iTrack)
				bReset = true;
		}

		if (m_bGood)
		{
			wstring wstrTitle = bReset ? L"iTunes" : m_ssSettings.wciTunesTitle;

			if (!bReset)
				ReplaceConstants(wstrTitle);

			if (wstrTitle != L"")
				SetWindowText(m_hWndITunes, wstrTitle.c_str());
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::Stabilize()
{
	//find itunes window
	m_hWndITunes = FindWindow(L"iTunes", L"iTunes");

	if (m_hWndITunes && NULL == GetWindow(m_hWndITunes, GW_CHILD))
		m_hWndITunes = FindWindowEx(NULL, m_hWndITunes, L"iTunes", L"iTunes");

	if (m_hWndITunes == NULL)
		m_hWndITunes = FindWindow(L"ITWindow", L"iTunes");

	try
	{
		if (m_hWndITunes == NULL)
		{
			if (iITunes) //m_bGood may not be correct
			{
				if (m_pIiTunesEventHandler)
				{
					m_pIiTunesEventHandler->ShutdownConnectionPoint();
					m_pIiTunesEventHandler->Release();
					m_pIiTunesEventHandler = NULL;
				}
				iITunes->Release();
			}
			iITunes = NULL;
			m_pIiTunesEventHandler = NULL;
			m_bGood = false;
		}
		else
		{
			if (iITunes) //m_bGood may not be correct
			{
				this->Disconnect();
				this->Connect(true);
			}
			else
			{
				this->Connect(true);
			}
		}
	}
	catch (...)
	{
		// If we got an exception, something is very, very wrong
		// Set disconnected state
		m_bGood = false;
		// Clear (possibly leaking!!) handles
		iITunes = NULL;
		m_pIiTunesEventHandler = NULL;
		m_hWndITunes = NULL;
	}

	return;
}

void CITunesCOM::UpdateIPod()
{
	if (IsStable())
		iITunes->UpdateIPod();

	return;
}

void CITunesCOM::KillArtworkTemp()
{
	if (wcscmp(m_wcArtTemp, L"") != 0 && m_bArtIsTemp)
	{
		m_bArtIsTemp = false;
		_wremove(m_wcArtTemp);
		wcscpy_s(m_wcArtTemp, MAX_PATH, L"");
	}
	else if (wcscmp(m_wcArtTemp, L"") != 0)
	{
		wcscpy_s(m_wcArtTemp, MAX_PATH, L"");
	}

	return;
}

void CITunesCOM::Delete()
{
	IITTrackPtr iTrack;
	IITFileOrCDTrackPtr iTrackFile;
	_bstr_t bstrPath;

	try
	{
		if (!IsStable())
			return;

		if (S_OK != iITunes->get_CurrentTrack(&iTrack))
			return;

		iTrackFile = IITFileOrCDTrackPtr(iTrack);
		if (NULL == iTrackFile)
			return;

		if (S_OK != iTrackFile->get_Location(bstrPath.GetAddress()))
			return;

		iTrack->Delete();

		if (m_ssSettings.bDeleteFromDisk)
			_wremove(bstrPath);
	}
	catch (...)
	{
	}
}

bool CITunesCOM::FindFolderArt()
{
	if (!m_ssSettings.bFolderArt || !IsStable())
		return false;

	IITTrackPtr iTrack;
	IITFileOrCDTrackPtr iTrackFile;
	bool bRet = false;
	_bstr_t bstrPath;
	wchar_t wcPath[MAX_PATH] = { 0 };

	if (S_OK != iITunes->get_CurrentTrack(&iTrack))
		return false;

	iTrackFile = IITFileOrCDTrackPtr(iTrack);
	if (NULL == iTrackFile)
		return false;

	if (S_OK != iTrackFile->get_Location(bstrPath.GetAddress()))
		return false;

	// strip off filename, try folder.jpg and folder.png
	wcscpy_s(wcPath, MAX_PATH, bstrPath);

	PathRemoveFileSpec(wcPath);
	PathAppend(wcPath, L"folder.jpg");

	if (TRUE == PathFileExists(wcPath))
	{
		// we got a path
		wcscpy_s(this->m_wcArtTemp, MAX_PATH + 1, wcPath);
		m_bArtIsTemp = false;
		bRet = true;
	}
	else
	{
		PathRemoveFileSpec(wcPath);
		PathAppend(wcPath, L"folder.png");
		if (TRUE == PathFileExists(wcPath))
		{
			// we got a path
			wcscpy_s(this->m_wcArtTemp, MAX_PATH + 1, wcPath);
			m_bArtIsTemp = false;
			bRet = true;
		}
	}

	return bRet;
}

bool CITunesCOM::DumpArtwork()
{
	bool bRet = false;
	HRESULT hRes = 0;
	long lCount = 0;
	IITTrackPtr iTrack;
	IITArtworkCollectionPtr iArtworkCollection;
	IITArtworkPtr iArtwork;
	ITArtworkFormat iArtworkFormat;
	wchar_t wcArtTemp[MAX_PATH + 1] = { 0 };
	_bstr_t bstrPath;

	try
	{
		_DBGLOG(L"Attempting to extract album art");
		if (!IsStable())
			return false;

		hRes = iITunes->get_CurrentTrack(&iTrack);

		if (hRes != S_OK || iTrack == NULL)
			return false;

		hRes = iTrack->get_Artwork(&iArtworkCollection);

		if (hRes != S_OK)
			return false;

		hRes = iArtworkCollection->get_Count(&lCount);
		if (hRes != S_OK)
			return false;

		for (long i = 1; i <= lCount; i++)
		{
			hRes = iArtworkCollection->get_Item(i, &iArtwork);
			if (hRes != S_OK)
				return false;

			hRes = iArtwork->get_Format(&iArtworkFormat);
			if (hRes == S_OK)
			{

				if (GetTempPath(MAX_PATH, wcArtTemp) != 0)
					PathAppend(wcArtTemp, L"itc_art");


				switch (iArtworkFormat)
				{
				case 0: //unknown
					wcscat_s(wcArtTemp, MAX_PATH + 1, L".tmp");
					break;
				case 1: //jpeg
					wcscat_s(wcArtTemp, MAX_PATH + 1, L".jpg");
					break;
				case 2: //png
					wcscat_s(wcArtTemp, MAX_PATH + 1, L".png");
					break;
				case 3: //bmp
					wcscat_s(wcArtTemp, MAX_PATH + 1, L".bmp");
					break;
				default:
					break;
				}

				bstrPath = wcArtTemp;
				hRes = iArtwork->SaveArtworkToFile(bstrPath);
				if (hRes == S_OK)
				{
					wcscpy_s(m_wcArtTemp, MAX_PATH + 1, wcArtTemp);
					m_bArtIsTemp = true;
					bRet = true;
					_DBGLOG(L"Album art extracted");
				}
			}
		}
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in DumpArtwork().");
	}

	return bRet;
}

void CITunesCOM::SetTrackNormalize(long lNormalize)
{
	HRESULT hRes = 0;
	IITTrackPtr iTrack;

	try
	{
		if (!IsStable())
			return;

		hRes = iITunes->get_CurrentTrack(&iTrack);
		if (S_OK != hRes || NULL == iTrack)
			return;

		iTrack->put_VolumeAdjustment(lNormalize);

		wchar_t wcLength[50] = { 0 };
		wsprintf(wcLength, L"%s: %d%%", L"Per-Track Volume Adjustment", lNormalize * 100 / 255);

		_ginterface->DisplayStatus(wcLength);
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::ShowHide()
{
	// Minimizing and restoring seem to be the only
	//   reliable thing to do.

	if (GetForegroundWindow() == m_hWndITunes)
		ShowWindowAsync(m_hWndITunes, SW_MINIMIZE);
	else
	{
		ShowWindowAsync(m_hWndITunes, SW_MINIMIZE);
		ShowWindowAsync(m_hWndITunes, SW_RESTORE);
		SetForegroundWindow(m_hWndITunes);
	}
}

void CITunesCOM::ShowCurrentInfo(int display_time, bool bUpdate)
{
	wstring wstrOSDText;
	HRESULT hRes;
	IITTrackPtr iTrack;

	try
	{
		if (!IsStable())
			return;

		hRes = iITunes->get_CurrentTrack(&iTrack);
		if (S_OK != hRes || NULL == iTrack)
			return;

		//replace line constants with info
		//if ( bUpdate )
		//	this->GetTrackInfo();
		wstrOSDText = m_ssSettings.wcOSDFormat;
		this->ReplaceConstants(wstrOSDText);

		if (!bUpdate)
			_ginterface->OverrideAlwaysUp(false);

		_ginterface->DisplayInfo((m_ssSettings.nDisplayLayout == 4 ? L"" : wstrOSDText), true, m_wcArtTemp, display_time, bUpdate);
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::FormatTitle(wstring *wstrFormat)
{
	this->ReplaceConstants(*wstrFormat, false);

	return;
}

void CITunesCOM::ReplaceConstants(wstring & wstrLine, bool bAllowImageRatings)
{
	if (!IsStable())
		return;

	try
	{
		Replace(wstrLine, L"%artist%", stiCurrent.wstrArtist);
		Replace(wstrLine, L"%album%", stiCurrent.wstrAlbum);
		Replace(wstrLine, L"%rating-text%", stiCurrent.wstrRating);
		if (0 == m_ssSettings.nRatingType || !bAllowImageRatings)
			Replace(wstrLine, L"%rating%", stiCurrent.wstrRating);
		Replace(wstrLine, L"%track%", stiCurrent.wstrTitle);
		Replace(wstrLine, L"%tags%", stiCurrent.wstrTags);
		Replace(wstrLine, L"%genre%", stiCurrent.wstrGenre);
		Replace(wstrLine, L"%comment%", stiCurrent.wstrComment);

		wchar_t wcLength[20] = { 0 };
		long lTrackPosition = 0;
		wsprintf(wcLength, L"%d:%.2d", stiCurrent.nTrackLength / 60, stiCurrent.nTrackLength % 60);
		Replace(wstrLine, L"%track_length%", wcLength);
		iITunes->get_PlayerPosition(&lTrackPosition);
		if (lTrackPosition > stiCurrent.nTrackLength)
			lTrackPosition = stiCurrent.nTrackLength;
		wsprintf(wcLength, L"%d:%.2d", lTrackPosition / 60, lTrackPosition % 60);
		Replace(wstrLine, L"%track_position%", wcLength);

		wsprintf(wcLength, L"%d kbps", stiCurrent.nBitRate);
		Replace(wstrLine, L"%bit_rate%", wcLength);

		wsprintf(wcLength, L"%d%%", stiCurrent.lNormalize * 100 / 255);
		Replace(wstrLine, L"%normalize%", wcLength);

		Replace(wstrLine, L"%year%", stiCurrent.wstrYear);

		Replace(wstrLine, L"%total_tracks%", stiCurrent.wstrTotalTracks);

		Replace(wstrLine, L"%composer%", stiCurrent.wstrComposer);

		Replace(wstrLine, L"%tracknumber%", stiCurrent.wstrTrackNum);

		if (!m_ssSettings.bAllowBlankLines)
			Replace(wstrLine, L"\r\n\r\n", L"\r\n");
	}
	catch (...) //an exception just means that we haven't gotten song info yet
	{
	}

	return;
}

long CITunesCOM::GetTimeLeftInTrack()
{
	if (!IsStable())
		return 0;

	long lTrackPosition = 0;

	iITunes->get_PlayerPosition(&lTrackPosition);
	if (lTrackPosition > stiCurrent.nTrackLength)
		lTrackPosition = stiCurrent.nTrackLength;

	return (stiCurrent.nTrackLength - lTrackPosition);
}

long CITunesCOM::GetTimeInTrack()
{
	if (!IsStable())
		return 0;

	long lTrackPosition = 0;

	iITunes->get_PlayerPosition(&lTrackPosition);

	return (lTrackPosition);
}

void CITunesCOM::Replace(wstring & wstrText, wstring wstrFind, wstring wstrReplace)
{
	size_t pos = 0;

	pos = wstrText.find(wstrFind);
	while (pos != wstring::npos)
	{
		wstrText.replace(pos, wstrFind.length(), wstrReplace);
		pos = wstrText.find(wstrFind);
	}

	return;
}

void CITunesCOM::SaveTags()
{
	wstring wstrTemp;
	IITTrackPtr iTrack;
	HRESULT hRes = NULL;

	vector<wstring>::iterator p = _wstrTags.begin();

	if (0 == _wstrTags.size())
		wstrTemp = L"";
	else
	{
		while (p != _wstrTags.end())
		{
			wstrTemp += L"*";
			wstrTemp += *p;
			wstrTemp += L" ";
			p++;
		}
	}

	try
	{
		if (IsStable())
		{
			hRes = iITunes->get_CurrentTrack(&iTrack);

			if (hRes == S_OK && iTrack != NULL)
			{
				_bstr_t bstrCom(wstrTemp.c_str());

				if (0 == m_ssSettings.nTagField)
					iTrack->put_Comment(bstrCom);
				else
					iTrack->put_Grouping(bstrCom);
			}
		}
	}
	catch (...)
	{
	}

}

void CITunesCOM::CommentToTags(wstring & wstrText)
{
	size_t pStart, pEnd = 0;
	wstring wstrTemp;
	_wstrTags.clear();

	pStart = wstrText.find(L"*");

	try
	{
		while (pStart != wstring::npos)
		{
			pEnd = wstrText.find(L" ", pStart);
			if (pEnd != wstring::npos)
			{
				wstrTemp += wstrText.substr(pStart + 1, pEnd - pStart);
				_wstrTags.push_back(wstrText.substr(pStart + 1, pEnd - 1 - pStart));
			}
			else //assume it's the end of the string
			{
				wstrTemp += wstrText.substr(pStart + 1, wstrText.length() - pStart);
				_wstrTags.push_back(wstrText.substr(pStart + 1, wstrText.length() - 1 - pStart));
			}
			pStart = wstrText.find(L"*", pEnd);
		}

		wstrText = wstrTemp;
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in CommentToTags()!");
	}


	return;
}

void CITunesCOM::SetSleepTimer(int nMinutes)
{
	HWND hWndITC = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");

	if (hWndITC != NULL)
	{
		SetTimer(hWndITC, 1320, 60000 * nMinutes, NULL);
		m_bSleepOn = true;
	}

	return;
}

void CITunesCOM::ClearSleepTimer()
{
	HWND hWndITC = FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl");

	if (hWndITC != NULL)
	{
		KillTimer(hWndITC, 1320);
		m_bSleepOn = false;
	}

	return;
}

void CITunesCOM::Jump(int nType, int nValue)
{
	HRESULT hRes = 0;
	IITTrackPtr iTrack;
	long lTrackLength = 0;
	long lTrackPos = 0;

	if (!IsStable())
		return;

	hRes = iITunes->get_CurrentTrack(&iTrack);
	if (S_OK != hRes || NULL == iTrack)
		return;

	iTrack->get_Duration(&lTrackLength);
	iITunes->get_PlayerPosition(&lTrackPos);

	switch (nType)
	{
	case 0:
		//second abs
		iITunes->put_PlayerPosition((long)nValue);
		break;
	case 1:
		//seconds rel
		iITunes->put_PlayerPosition(lTrackPos + (long)nValue);
		break;
	case 2:
		//percent abs
		iITunes->put_PlayerPosition(nValue * lTrackLength / 100);
		break;
	default:
		//percent rel
		iITunes->put_PlayerPosition(nValue * (lTrackLength - lTrackPos) / 100);
		break;
	}
}

void CITunesCOM::GetTrackInfo()
{
	HRESULT hRes = 0;
	long lTmp, lBitRate, lTrackLength, lRating, lNormalize;

	IITTrackPtr iTrack;
	_bstr_t bstrComposer;
	_bstr_t bstrArtist;
	_bstr_t bstrTitle;
	_bstr_t bstrAlbum;
	_bstr_t bstrComment;
	_bstr_t bstrGenre;
	_bstr_t bstrStreamTitle;
	_bstr_t bstrGrouping;

	try
	{
		if (!IsStable())
			return;

		hRes = iITunes->get_CurrentTrack(&iTrack);
		if (S_OK != hRes || NULL == iTrack)
			return;

		// Read data from COM interface
		iTrack->get_Artist(bstrArtist.GetAddress());
		iITunes->get_CurrentStreamTitle(bstrStreamTitle.GetAddress());
		iTrack->get_Name(bstrTitle.GetAddress());
		iTrack->get_Album(bstrAlbum.GetAddress());
		iTrack->get_Genre(bstrGenre.GetAddress());
		iTrack->get_Comment(bstrComment.GetAddress());
		iTrack->get_Grouping(bstrGrouping.GetAddress());
		iTrack->get_Composer(bstrComposer.GetAddress());
		iTrack->get_Rating(&lRating);

		if (iTrack->get_TrackCount(&lTmp) == S_OK && lTmp != 0)
			stiCurrent.wstrTotalTracks = to_wstring(lTmp);
		else
			stiCurrent.wstrTotalTracks = (m_ssSettings.bOSDShowMissingTags ? L"?" : L"");
		if (iTrack->get_TrackNumber(&lTmp) == S_OK && lTmp != 0)
			stiCurrent.wstrTrackNum = to_wstring(lTmp);
		else
			stiCurrent.wstrTrackNum = (m_ssSettings.bOSDShowMissingTags ? L"?" : L"");
		if (iTrack->get_Year(&lTmp) == S_OK && lTmp != 0)
			stiCurrent.wstrYear = to_wstring(lTmp);
		else
			stiCurrent.wstrYear = (m_ssSettings.bOSDShowMissingTags ? L"?" : L"");

		if (iTrack->get_Duration(&lTrackLength) != S_OK) { lTrackLength = 0; }
		if (iTrack->get_BitRate(&lBitRate) != S_OK) { lBitRate = 0; }
		if (iTrack->get_VolumeAdjustment(&lNormalize) != S_OK) { lNormalize = 0; }

		if (!bstrStreamTitle)
		{
			(!bstrArtist == false) ? stiCurrent.wstrArtist = bstrArtist : stiCurrent.wstrArtist = (m_ssSettings.bOSDShowMissingTags ? L"(None)" : L"");
			(!bstrTitle == false) ? stiCurrent.wstrTitle = bstrTitle : stiCurrent.wstrTitle = (m_ssSettings.bOSDShowMissingTags ? L"(None)" : L"");
		}
		else
		{
			stiCurrent.wstrTitle = bstrStreamTitle;
			stiCurrent.wstrArtist = (m_ssSettings.bOSDShowMissingTags ? L"(None)" : L"");
		}

		(!bstrComposer == false) ? stiCurrent.wstrComposer = bstrComposer : stiCurrent.wstrComposer = (m_ssSettings.bOSDShowMissingTags ? L"(None)" : L"");
		(!bstrAlbum == false) ? stiCurrent.wstrAlbum = bstrAlbum : stiCurrent.wstrAlbum = (m_ssSettings.bOSDShowMissingTags ? L"(None)" : L"");
		(!bstrGenre == false) ? stiCurrent.wstrGenre = bstrGenre : stiCurrent.wstrGenre = (m_ssSettings.bOSDShowMissingTags ? L"(None)" : L"");

		if (0 == m_ssSettings.nTagField && false == !bstrComment)
			stiCurrent.wstrTags = bstrComment;
		else if (1 == m_ssSettings.nTagField && false == !bstrGrouping)
			stiCurrent.wstrTags = bstrGrouping;
		else
			stiCurrent.wstrTags = L"";

		if (false == !bstrComment)
			stiCurrent.wstrComment = bstrComment;
		else
			stiCurrent.wstrComment = L"";

		stiCurrent.lRating = lRating;
		MakeRatingsText(lRating, &stiCurrent.wstrRating);

		stiCurrent.nBitRate = lBitRate;
		stiCurrent.nTrackLength = lTrackLength;
		stiCurrent.lNormalize = lNormalize;
	}
	catch (...)
	{
		_RPTFW0(_CRT_WARN, L"Caught exception in GetTrackInfo().");
	}

	return;
}

void CITunesCOM::MakeRatingsText(long lRating, wstring *wstrRating)
{
	if (lRating == 0)
	{
		*wstrRating = (m_ssSettings.bOSDShowMissingTags ? L"(none)" : L"");
		return;
	}

	long lWhole = (lRating - (lRating % 20)) / 20;
	lRating = lRating - lWhole * 20;
	long lHalf = (lRating - (lRating % 10)) / 10;
	long lEmpty = 5 - lWhole - lHalf;

	// Clear rating before building
	*wstrRating = L"";

	for (int i = 0; i < lWhole; i++)
	{
		*wstrRating += m_ssSettings.wcRatingFullChar;
		*wstrRating += L" ";
	}
	for (int i = 0; i < lHalf; i++)
	{
		*wstrRating += m_ssSettings.wcRatingHalfChar;
		*wstrRating += L" ";
	}
	if (m_ssSettings.wcRatingEmptyChar != 0)
	{
		for (int i = 0; i < lEmpty; i++)
		{
			*wstrRating += m_ssSettings.wcRatingEmptyChar;
			*wstrRating += L" ";
		}
	}

	// Remove ending space
	wstrRating->resize(wstrRating->size() - 1);
}

void CITunesCOM::CycleRepeatMode()
{
	if (!IsStable())
		return;

	try
	{
		HRESULT hRes = 0;
		ITPlaylistRepeatMode itMode;
		wchar_t wcLength[50] = { 0 };

		IITPlaylistPtr iPlaylist;

		hRes = iITunes->get_CurrentPlaylist(&iPlaylist);
		if (hRes != S_OK)
			return;

		hRes = iPlaylist->get_SongRepeat(&itMode);
		if (hRes != S_OK)
			return;

		switch (itMode)
		{
		case ITPlaylistRepeatModeOff:
			iPlaylist->put_SongRepeat(ITPlaylistRepeatModeOne);
			_stprintf_s(wcLength, 50, L"%s\n%s", L"Repeat Mode", L"Repeat One");
			_ginterface->DisplayStatus(wcLength);
			break;
		case ITPlaylistRepeatModeOne:
			if (m_ssSettings.bOnlyToggleRepeatOne)
			{
				iPlaylist->put_SongRepeat(ITPlaylistRepeatModeOff);
				_stprintf_s(wcLength, 50, L"%s\n%s", L"Repeat Mode", L"Repeat Off");
			}
			else
			{
				iPlaylist->put_SongRepeat(ITPlaylistRepeatModeAll);
				_stprintf_s(wcLength, 50, L"%s\n%s", L"Repeat Mode", L"Repeat All");
			}

			_ginterface->DisplayStatus(wcLength);
			break;
		case ITPlaylistRepeatModeAll:
			iPlaylist->put_SongRepeat(ITPlaylistRepeatModeOff);
			_stprintf_s(wcLength, 50, L"%s\n%s", L"Repeat Mode", L"Repeat Off");
			_ginterface->DisplayStatus(wcLength);
			break;
		}
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::ToggleShuffle()
{
	if (!IsStable())
		return;

	try
	{
		HRESULT hRes = 0;
		VARIANT_BOOL bShuffle;
		wchar_t wcMsg[50] = { 0 };
		wstring wstrDisplay;

		IITPlaylistPtr iPlaylist;

		hRes = iITunes->get_CurrentPlaylist(&iPlaylist);
		if (hRes != S_OK)
			return;

		hRes = iPlaylist->get_Shuffle(&bShuffle);
		if (hRes != S_OK)
			return;

		if (bShuffle == VARIANT_TRUE)
		{
			bShuffle = VARIANT_FALSE;
			iPlaylist->put_Shuffle(bShuffle);

			swprintf_s(wcMsg, L"%s", L"Off");
		}
		else
		{
			bShuffle = VARIANT_TRUE;
			iPlaylist->put_Shuffle(bShuffle);

			swprintf_s(wcMsg, L"%s", L"On");
		}
		wstrDisplay = L"Shuffle";
		wstrDisplay += L"\n";
		wstrDisplay += wcMsg;
		_ginterface->DisplayStatus(wstrDisplay);
	}
	catch (...)
	{
	}

	return;
}

void CITunesCOM::ToggleTrackEnable()
{
	if (!IsStable())
		return;

	try
	{
		HRESULT hRes = 0;
		VARIANT_BOOL bEnable;
		wchar_t wcMsg[50] = { 0 };

		IITTrackPtr iTrack;

		hRes = iITunes->get_CurrentTrack(&iTrack);
		if (hRes != S_OK)
			return;

		hRes = iTrack->get_Enabled(&bEnable);
		if (hRes != S_OK)
			return;

		if (bEnable == VARIANT_TRUE)
		{
			bEnable = VARIANT_FALSE;
			iTrack->put_Enabled(bEnable);

			_ginterface->DisplayStatus(L"Track Disabled");
		}
		else
		{
			bEnable = VARIANT_TRUE;
			iTrack->put_Enabled(bEnable);

			_ginterface->DisplayStatus(L"Track Enabled");
		}
	}
	catch (...)
	{
	}

	return;
}

//nType: 0 == artist, 1 == album
//nDir: 0 == next, 1 == prev
//return: 0-success, 1-failure, 2-no itunes, 3-on shuffle
int CITunesCOM::AdvancedSeek(int nType, int nDir)
{
	//get out now, we'll just have an exception otherwise
	if (m_hWndITunes == NULL)
		return 2;

	try
	{
		if (!IsStable())
			return 1;

		HRESULT hRes = 0;
		BSTR bstrText = 0;
		wstring wstrText, wstrTemp;

		long lIndex = 0;
		IITTrack* iTrack = NULL;
		IITPlaylist* iPlaylist = NULL;
		IITTrackCollection* iTrackColl = NULL;

		hRes = iITunes->get_CurrentPlaylist(&iPlaylist);
		if (hRes != S_OK)
			return 1;

		hRes = iITunes->get_CurrentTrack(&iTrack);
		if (hRes != S_OK)
			return 1;

		hRes = iTrack->get_PlayOrderIndex(&lIndex);
		if (hRes != S_OK)
			return 1;

		hRes = iPlaylist->get_Tracks(&iTrackColl);

		if (hRes == S_OK)
		{
			if (nType == 0)
				iTrack->get_Artist(&bstrText);

			if (nType == 1)
				iTrack->get_Album(&bstrText);

			if (bstrText)
				wstrText = bstrText;
			else
				wstrText = L"(None)";

			iTrack->Release();

			SysFreeString(bstrText);
			bstrText = 0;
		}

		wstrTemp = wstrText; //start it off

		iTrack = NULL;

		while (wstrText.compare(wstrTemp) == 0)
		{
			if (nDir == 0)
				lIndex++;
			else
				lIndex--;

			hRes = iTrackColl->get_ItemByPlayOrder(lIndex, &iTrack);
			if (iTrack == NULL)
			{
				iTrackColl->Release();
				iPlaylist->Release();
				return 1;
			}

			if (hRes == S_OK)
			{
				if (nType == 0)
					iTrack->get_Artist(&bstrText);

				if (nType == 1)
					iTrack->get_Album(&bstrText);

				if (bstrText)
					wstrTemp = bstrText;
				else
					wstrTemp = L"(None)";

				iTrack->Release();

				SysFreeString(bstrText);
				bstrText = 0;
			}
		}

		if (nDir == 1) //if in reverse, do it again
		{
			wstrText = wstrTemp;

			while (wstrText.compare(wstrTemp) == 0)
			{
				lIndex--;

				hRes = iTrackColl->get_ItemByPlayOrder(lIndex, &iTrack);

				if (iTrack == NULL) //only happens if we were on the 2nd ___ and reached beg of playlist
				{
					//iTrack->Release();
					lIndex++;
					hRes = iTrackColl->get_ItemByPlayOrder(lIndex, &iTrack);

					if (iTrack != NULL)
					{
						iTrack->Play();
						iTrack->Release();
					}

					iTrackColl->Release();
					iPlaylist->Release();
					return 0;
				}

				if (hRes == S_OK)
				{
					if (nType == 0)
						iTrack->get_Artist(&bstrText);

					if (nType == 1)
						iTrack->get_Album(&bstrText);

					if (bstrText)
						wstrTemp = bstrText;
					else
						wstrTemp = L"(None)";

					iTrack->Release();

					SysFreeString(bstrText);
					bstrText = 0;
				}
			}

			lIndex++;
		}

		hRes = iTrackColl->get_ItemByPlayOrder(lIndex, &iTrack);

		if (iTrack != NULL)
		{
			iTrack->Play();
			iTrack->Release();
		}

		iTrackColl->Release();
		iPlaylist->Release();
	}
	catch (...)
	{
	}

	return 0;
}