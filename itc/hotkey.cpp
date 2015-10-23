#include "itunescominterface.h"
#include "hotkey.h"
#include "memcheck.h"

LRESULT ProcessHotkey(HWND hWnd, WPARAM wParam)
{
	// As long as we're not quitting, start iTunes if we're supposed to
	if (HK_QUIT != wParam && m_ssSettings.bStartOnHotkeys && !m_ITCom->IsConnected())
	{
		m_ITCom->Connect(true);

		do
		{
			Sleep(100);
		} while (!m_ITCom->IsConnected());

		PostMessage(hWnd, WM_STARTWITHITUNES, 0, 0);
	}

	switch (wParam)
	{
	case HK_PREV:
		if (m_bTrackChangeAllowed)
		{
			m_ITCom->PreviousTrack();
			m_bTrackChangeAllowed = false;
			SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait x secs before we do it again
		}
		break;
	case HK_PLAYPAUSE:
	{
		m_ITCom->PlayPause();
	}
	break;
	case HK_STOP:
	{
		m_ITCom->Stop();
	}
	break;
	case HK_NEXT:
		if (m_bTrackChangeAllowed)
		{
			m_ITCom->NextTrack();
			m_bTrackChangeAllowed = false;
			SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait before we do it again
		}
		break;
	case HK_DELETE:
	{
		if (!m_ssSettings.bPromptOnDelete || IDYES == MessageBox(hWnd, L"Delete Track", L"Are you sure you wish to delete this track?", MB_ICONQUESTION | MB_YESNO | MB_SYSTEMMODAL))
			m_ITCom->Delete();
	}
	break;
	case HK_SHOWCUR:
	{
		if (!m_ITCom->IsConnected())
			break;

		if (_ginterface->IsDisplayVisible())
		{
			_ginterface->OverrideAlwaysUp(true);
			_ginterface->DisplayHide();
		}
		else
			m_ITCom->ShowCurrentInfo();
	}
	break;

	case HK_NEXTARTIST:
	{
		if (m_bTrackChangeAllowed)
		{
			int nRet = m_ITCom->AdvancedSeek(0, 0);
			if (nRet == 0)
			{
				m_bTrackChangeAllowed = false;
				SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait before we do it again
			}
		}
	}
	break;

	case HK_PREVARTIST:
	{
		if (m_bTrackChangeAllowed)
		{
			int nRet = m_ITCom->AdvancedSeek(0, 1);
			if (nRet == 0)
			{
				m_bTrackChangeAllowed = false;
				SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait before we do it again
			}
		}
	}
	break;

	case HK_NEXTALBUM:
	{
		if (m_bTrackChangeAllowed)
		{
			int nRet = m_ITCom->AdvancedSeek(1, 0);
			if (nRet == 0)
			{
				m_bTrackChangeAllowed = false;
				SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait before we do it again
			}
		}
	}
	break;

	case HK_TOGGLEENABLE:
	{
		m_ITCom->ToggleTrackEnable();
	}
	break;

	case HK_PREVALBUM:
	{
		if (m_bTrackChangeAllowed)
		{
			int nRet = m_ITCom->AdvancedSeek(1, 1);
			if (nRet == 0)
			{
				m_bTrackChangeAllowed = false;
				SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait before we do it again
			}
		}
	}
	break;

	case HK_VOLUMEDOWN:
	{
		m_ITCom->VolumeDown(m_ssSettings.nVolumeChgInt);
	}
	break;

	case HK_STOPAFTERCURRENT:
	{
		m_ITCom->StopAfterCurrent();
	}
	break;

	case HK_VOLUMEUP:
	{
		m_ITCom->VolumeUp(m_ssSettings.nVolumeChgInt);
	}
	break;

	case HK_VOLUMEMUTE:
	{
		m_ITCom->VolumeMute();
	}
	break;

	case HK_RATEZERO:
	{
		m_ITCom->SetTrackRating(0);
	}
	break;

	case HK_RATEONE:
	{
		m_ITCom->SetTrackRating(20);
	}
	break;

	case HK_RATETWO:
	{
		m_ITCom->SetTrackRating(40);
	}
	break;

	case HK_RATETHREE:
	{
		m_ITCom->SetTrackRating(60);
	}
	break;

	case HK_RATEFOUR:
	{
		m_ITCom->SetTrackRating(80);
	}
	break;

	case HK_RATEFIVE:
	{
		m_ITCom->SetTrackRating(100);
	}
	break;

	case HK_JUMPONE:
	{
		m_ITCom->Jump(m_ssSettings.nJumpOneType, m_ssSettings.nJumpOneValue);
	}
	break;

	case HK_JUMPTWO:
	{
		m_ITCom->Jump(m_ssSettings.nJumpTwoType, m_ssSettings.nJumpTwoValue);
	}
	break;

	case HK_RESTARTTRACK:
	{
		m_ITCom->Jump(0, 0);
	}
	break;

	case HK_TAG:
	{
		DoTagStuff();
	}
	break;

	case HK_QUICKPLAYLIST:
	{
		m_ITCom->QuickPlaylist(m_ssSettings.wcQuickPlaylist);
	}
	break;

	case HK_SONGSEARCH:
	{
		SendMessage(m_hWndSongSearchListBox, LB_RESETCONTENT, NULL, NULL);
		SendMessage(m_hWndSongSearchListBox, LB_ADDSTRING, NULL, (LPARAM)L"Begin typing to search");
		SendMessage(m_hWndSongSearchListBox, LB_ADDSTRING, NULL, (LPARAM)L"Press [Enter] to play a result");
		SendMessage(m_hWndSongSearchListBox, LB_ADDSTRING, NULL, (LPARAM)L"Press [Shift]+[Enter] to play in iTunes DJ");

		ShowWindow(m_hWndSongSearch, SW_SHOW);
		SetForegroundWindow(m_hWndSongSearch);

		SendMessage(m_hWndSongSearchTextBox, CB_SETCUEBANNER, 0, (LPARAM)L"Begin typing to search");
	}
	break;

	case HK_PLAYLISTSEARCH:
	{
		// get list of playlists for searching
		vPlaylists.clear();
		m_ITCom->PlaylistSearch(&vPlaylists);

		SendMessage(m_hWndPlaylistSearchListBox, LB_RESETCONTENT, NULL, NULL);
		vector<wstring>::iterator p = vPlaylists.begin();
		while (p != vPlaylists.end())
		{
			SendMessage(m_hWndPlaylistSearchListBox, LB_ADDSTRING, NULL, (LPARAM)p->c_str());
			p++;
		}

		ShowWindow(m_hWndPlaylistSearch, SW_SHOW);
		SetForegroundWindow(m_hWndPlaylistSearchTextBox);
		SendMessage(m_hWndPlaylistSearchTextBox, CB_SETCUEBANNER, 0, (LPARAM)L"Begin typing to search");
	}
	break;

	case HK_SLEEPTIMER:
	{
		inputView ivPage;
		inputItem *ivItems = new inputItem[2];

		if (m_ITCom->m_bSleepOn)
		{
			ivItems[0].wstrTitle = L"Reset";
		}
		else
		{
			ivItems[0].wstrTitle = L"Set";
		}
		ivItems[0].nID = 0;
		ivItems[0].nType = 0;
		ivItems[1].wstrTitle = L"Cancel";
		ivItems[1].nID = 1;
		ivItems[1].nType = 0;

		ivPage.iiList = ivItems;
		ivPage.nItems = 2;
		ivPage.nPos = 0;
		ivPage.wstrTitle = L"Sleep Timer";

		int nRet = inputWindow->DisplayInput(ivPage);

		switch (nRet)
		{
		case -1: //intake cancelled
			break;
		case 0:
		{
			ivItems[0].wstrTitle = L"?";
			ivItems[0].nType = 1; //input box
			ivItems[0].nValue = m_ssSettings.nSleepTimer;
			ivItems[0].nMax = 120;
			ivItems[0].nMin = 1;
			ivPage.wstrTitle = L"How many minutes?";

			int nRet = inputWindow->DisplayInput(ivPage);

			if (nRet == 0)
			{
				m_ssSettings.nSleepTimer = ivPage.iiList[0].nValue;

				//now, what kind of sleep?

				delete[] ivItems;
				ivItems = NULL;

				inputItem *ivItems = new inputItem[1];
				wstring *wstrValues = new wstring[7];

				wstrValues[0] = L"Pause iTunes";
				wstrValues[1] = L"Close iTunes";
				wstrValues[2] = L"Shutdown Computer";
				wstrValues[3] = L"Suspend Computer";
				wstrValues[4] = L"Hibernate Computer";
				wstrValues[5] = L"Pause iTunes/Lock Computer";
				wstrValues[6] = L"Lock Computer";

				ivItems[0].wstrTitle = L""; // doesn't matter
				ivItems[0].nID = 1;
				ivItems[0].nType = 2;
				ivItems[0].nMin = 0;
				ivItems[0].nMax = 6;
				ivItems[0].nValue = m_ssSettings.nSleepType;
				ivItems[0].wstrSelect = wstrValues;

				ivPage.iiList = ivItems;
				ivPage.nItems = 1;
				ivPage.nPos = 0;
				ivPage.wstrTitle = L"Sleep Timer Type";

				int nRet = inputWindow->DisplayInput(ivPage);

				if (nRet != -1)
				{
					m_ssSettings.nSleepType = ivPage.iiList[0].nValue;

					if (m_ITCom->m_bSleepOn)
					{
						_ginterface->DisplayStatus(L"Sleep Timer Reset");
					}
					else
					{
						_ginterface->DisplayStatus(L"Sleep Timer Set");
					}

					m_ITCom->SetSleepTimer(m_ssSettings.nSleepTimer);
				}

				delete[] ivItems;
				ivItems = NULL;
				delete[] wstrValues;
			}
			break;
		}
		case 1:
			if (m_ITCom->m_bSleepOn)
			{
				_ginterface->DisplayStatus(L"Sleep Timer Cancelled");
			}

			m_ITCom->ClearSleepTimer();
			break;
		}

		if (ivItems != NULL)
			delete[] ivItems;
	}
	break;

	case HK_REPEATMODE:
	{
		m_ITCom->CycleRepeatMode();
	}
	break;

	case HK_UPDATEIPOD:
	{
		m_ITCom->UpdateIPod();
	}
	break;

	case HK_SHUFFLE:
	{
		m_ITCom->ToggleShuffle();
	}
	break;

	case HK_SHOWHIDEDISPLAY:
	{
		if (_ginterface->IsDisplayVisible())
			_ginterface->DisplayHide();
		else
			_ginterface->DisplayShow();
	}
	break;

	case HK_RATINGINC:
	{
		long lRating = m_ITCom->GetTrackRating();

		if (lRating <= (100 - m_ssSettings.nRatingDelta))
			lRating += m_ssSettings.nRatingDelta;
		else
			lRating = 100;

		m_ITCom->SetTrackRating(lRating);
	}
	break;

	case HK_RATINGDEC:
	{
		long lRating = m_ITCom->GetTrackRating();

		if (lRating >= m_ssSettings.nRatingDelta)
			lRating -= m_ssSettings.nRatingDelta;
		else
			lRating = 0;

		m_ITCom->SetTrackRating(lRating);
	}
	break;
	case HK_COPYSONGINFO:
	{
		LPTSTR  lptstrCopy;
		HGLOBAL hglbCopy;
		wstring wstrData = m_ssSettings.wcCSIFormat;

		m_ITCom->FormatTitle(&wstrData);

		if (!OpenClipboard(hWnd))
		{
			MessageBox(hWnd, L"iTunesControl was unable to open the clipboard!", L"Copy Song Info Error", MB_OK | MB_ICONERROR);
			break;
		}
		EmptyClipboard();

		hglbCopy = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (wstrData.length() + 1) * 2);
		if (hglbCopy == NULL)
		{
			MessageBox(hWnd, L"iTunesControl was unable to open the clipboard!", L"Copy Song Info Error", MB_OK | MB_ICONERROR);
			CloseClipboard();
			break;
		}

		lptstrCopy = static_cast<LPTSTR>(GlobalLock(hglbCopy));
		wmemcpy(lptstrCopy, wstrData.c_str(), wstrData.length());
		GlobalUnlock(hglbCopy);

		if (NULL == SetClipboardData(CF_UNICODETEXT, hglbCopy))
		{
			MessageBox(hWnd, L"iTunesControl was unable to open the clipboard!", L"Copy Song Info Error", MB_OK | MB_ICONERROR);
		}

		CloseClipboard();
	}
	break;

	case HK_PLAYINITUNESDJ:
	{
		wchar_t wcMsg[50] = { 0 };

		m_ITCom->PlayIniTunesDJ();
		_ginterface->DisplayStatus(L"Current track added to iTunes DJ playlist");
	}
	break;

	case HK_SHOWRATE:
	{
		wstring wstrRating = L"%rating%";
		m_ITCom->DisplayFormattedText(&wstrRating);
	}
	break;

	case HK_SETRATING:
	{
		inputView ivPage;
		inputItem *ivItems = new inputItem[1];
		wstring *wstrValues = new wstring[11];

		wstrValues[0] = L"(None)";
		wstrValues[1] = L"½";
		wstrValues[2] = L"*";
		wstrValues[3] = L"* ½";
		wstrValues[4] = L"* *";
		wstrValues[5] = L"* * ½";
		wstrValues[6] = L"* * *";
		wstrValues[7] = L"* * * ½";
		wstrValues[8] = L"* * * *";
		wstrValues[9] = L"* * * * ½";
		wstrValues[10] = L"* * * * *";

		ivItems[0].wstrTitle = L""; //doesn't matter
		ivItems[0].nID = 1;
		ivItems[0].nType = 2;
		ivItems[0].nMin = 0;
		ivItems[0].nMax = 10;
		ivItems[0].nValue = m_ITCom->GetTrackRating() / 10;
		ivItems[0].wstrSelect = wstrValues;

		ivPage.iiList = ivItems;
		ivPage.nItems = 1;
		ivPage.nPos = 0;
		ivPage.wstrTitle = L"Set Track Rating";

		int nRet = inputWindow->DisplayInput(ivPage);

		if (nRet != -1)
		{
			m_ITCom->SetTrackRating(ivPage.iiList[0].nValue * 10);
		}

		delete[] ivItems;
		delete[] wstrValues;
	}
	break;

	case HK_SHOWSETTINGS:
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		TCHAR szPath[MAX_PATH];

		GetModuleFileName(NULL, szPath, MAX_PATH);
		PathRemoveFileSpec(szPath);
		PathAppend(szPath, L"\\config.exe");

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		CreateProcess(szPath, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	break;

	case HK_SHOWHIDE:
	{
		m_ITCom->ShowHide();
	}
	break;

	case HK_FASTFORWARD:
	{
		if (m_bTrackChangeAllowed)
		{
			m_ITCom->FastForward(5);
			m_bTrackChangeAllowed = false;
			SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait x secs before we do it again
		}
	}
	break;

	case HK_REWIND:
	{
		if (m_bTrackChangeAllowed)
		{
			m_ITCom->Rewind(5);
			m_bTrackChangeAllowed = false;
			SetTimer(hWnd, 2, m_ssSettings.nTrackChangeDelay, NULL); //wait x secs before we do it again
		}
	}
	break;

	case HK_NORMALIZEUP:
	{
		long lNormalize = m_ITCom->GetTrackNormalize();

		if (lNormalize + m_ssSettings.nVolumeChgInt > 255)
			lNormalize = 255;
		else
			lNormalize += m_ssSettings.nVolumeChgInt;

		m_ITCom->SetTrackNormalize(lNormalize);
	}
	break;

	case HK_NORMALIZEDOWN:
	{
		long lNormalize = m_ITCom->GetTrackNormalize();

		if (lNormalize - m_ssSettings.nVolumeChgInt < -255)
			lNormalize = -255;
		else
			lNormalize -= m_ssSettings.nVolumeChgInt;

		m_ITCom->SetTrackNormalize(lNormalize);
	}
	break;

	case HK_NORMALIZEZERO:
	{
		m_ITCom->SetTrackNormalize(0);
	}
	break;

	case HK_QUIT:
	{
		try
		{
			if (!m_ITCom->IsConnected())
			{
				PostQuitMessage(0);
				break;
			}

			if (m_ssSettings.bQuitiTunesOnExit)
				if (m_ssSettings.bPromptQuitiTunes)
				{
					int nResult = MessageBox(FindWindowEx(HWND_MESSAGE, NULL, NULL, L"iTunesControl"), L"Quit iTunes?", L"iTunesControl",
						MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
					if (nResult == IDYES)
					{
						m_ITCom->Quit();
					}
					else if (nResult == IDCANCEL)
						break;
				}
				else
				{
					m_ITCom->Quit();
				}

			PostQuitMessage(0);
		}
		catch (...)
		{
			//ignore, we're quiting anyway
			_RPTFW0(_CRT_WARN, L"Caught exception while trying to quit.");
			PostQuitMessage(0);
		}
	}
	break;
	}

	return 0;
}

bool ShowAddTags()
{
	inputView ivPage;
	inputItem *ivItems = NULL;

	ivItems = new inputItem[1];
	ivItems[0].nID = 1;
	ivItems[0].nType = 3;
	ivItems[0].wstrTitle = L"";

	ivPage.iiList = ivItems;
	ivPage.nItems = 1;
	ivPage.nPos = 0;
	ivPage.wstrTitle = L"Add Tag";

	int nRet = inputWindow->DisplayInput(ivPage);

	switch (nRet)
	{
	case -1: //intake cancelled
		break;
	case 0: //set a tag
		m_ITCom->_wstrTags.push_back(ivItems[0].wstrTitle);
		m_ITCom->SaveTags();
		break;
	}

	delete[] ivItems;

	if (nRet == -1)
		return false;

	return true;
}

bool ShowAllTags()
{
	inputView ivPage;
	inputItem *ivItems = NULL;

	ivItems = new inputItem[m_ITCom->_wstrTags.size() + 1];
	vector<wstring>::iterator p = m_ITCom->_wstrTags.begin();
	int i = 0;

	if (0 == m_ITCom->_wstrTags.size())
		return true;

	while (p != m_ITCom->_wstrTags.end())
	{
		ivItems[i].nID = i;
		ivItems[i].nType = 3;
		ivItems[i].wstrTitle = *p;
		i++;
		p++;
	}

	ivItems[i].nID = i;
	ivItems[i].nType = 0;
	ivItems[i].wstrTitle = L"Add Tag";

	ivPage.iiList = ivItems;
	ivPage.nItems = i + 1;
	ivPage.nPos = 0;
	ivPage.wstrTitle = L"Tags";

	int nRet = inputWindow->DisplayInput(ivPage);

	if (nRet == i) //add tag
	{
		return true;
	}
	else if (nRet != -1) // it's a tag
	{
		if (MessageBox(m_hWndInput, L"Delete tag?", L"Remove Tag", MB_YESNO | MB_SETFOREGROUND) == IDYES)
		{
			m_ITCom->_wstrTags.erase(m_ITCom->_wstrTags.begin() + nRet);
		}
	}

	delete[] ivItems;

	return false;
}

void DoTagStuff()
{
	if (0 == m_ITCom->_wstrTags.size()) //no tags, "add tag" intake
	{
		if (ShowAddTags())
			DoTagStuff();
	}
	else
	{
		if (ShowAllTags())
			DoOtherTagStuff();
	}
}

void DoOtherTagStuff()
{
	if (ShowAddTags())
		DoTagStuff();
}