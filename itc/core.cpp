#include "core.h"
#include "memcheck.h"

void SetState(bool bConnected)
{
	wchar_t wcDisplay[100] = { 0 };
	bool bShowInTray = m_ssSettings.bShowInTray;

	if (bConnected && bShowInTray)
	{
		swprintf_s(wcDisplay, 100, L"iTunesControl %s\n%s", PRETTYVERSION, L"Connected");
		st.SetText(wcDisplay);
		st.SetIcon(LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LOGO)));
	}
	else if (!bConnected && bShowInTray)
	{
		swprintf_s(wcDisplay, 100, L"iTunesControl %s\n%s", PRETTYVERSION, L"Not Connected");
		st.SetText(wcDisplay);
		st.SetIcon(LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_XLOGO)));
	}

	return;
}

void PowerControl(int nType)
{
	HANDLE hToken;              // handle to process token 
	TOKEN_PRIVILEGES tkp;       // pointer to token structure 
	BOOL fResult;               // system shutdown flag 

	// Get the current process token handle so we can get shutdown privilege.
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;

	// Get the LUID for shutdown privilege. 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get shutdown privilege for this process. 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	// Cannot test the return value of AdjustTokenPrivileges. 
	if (GetLastError() != ERROR_SUCCESS)
		return;

	if (0 == nType)
	{
		// Display the shutdown dialog box and start the countdown.
		fResult = InitiateSystemShutdown(
			NULL,    // shut down local computer 
			NULL,   // message for user
			0,      // time-out period 
			FALSE,   // ask user to close apps 
			FALSE);   // reboot after shutdown 
	}
	else if (1 == nType)
	{
		SetSystemPowerState(TRUE, FALSE);
	}
	else if (2 == nType)
	{
		SetSystemPowerState(FALSE, FALSE);
	}

	// Disable shutdown privilege. 
	tkp.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	return;
}

SysTray::SysTray()
{
	myActive = false;
	memset(&myNID, 0, sizeof(NOTIFYICONDATA));
}

SysTray::~SysTray()
{
	if (myActive)
		Shell_NotifyIcon(NIM_DELETE, &myNID);
}

void SysTray::Create(HWND hWndOwner, TCHAR * szTip, HICON hIcon, UINT uID, UINT uCBMsg)
{
	if (myActive)
		return;

	memset(&myNID, 0, sizeof(NOTIFYICONDATA));

	myNID.cbSize = sizeof(NOTIFYICONDATA);
	myNID.uID = uID;
	myNID.hIcon = hIcon;
	myNID.hWnd = hWndOwner;
	wcscpy_s(myNID.szTip, 128, szTip);
	myNID.uCallbackMessage = uCBMsg;
	myNID.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
	myNID.uVersion = NOTIFYICON_VERSION;

	Shell_NotifyIcon(NIM_ADD, &myNID);

	myActive = true;

	return;
}

void SysTray::ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags)
{
	if (!myActive)
		return;

	myNID.cbSize = sizeof(NOTIFYICONDATA);
	myNID.uFlags = NIF_INFO;
	myNID.uTimeout = uTimeout;
	myNID.dwInfoFlags = dwInfoFlags;
	wcscpy_s(myNID.szInfo, 256, szMsg);
	wcscpy_s(myNID.szInfoTitle, 64, szTitle);

	Shell_NotifyIcon(NIM_MODIFY, &myNID);

	return;
}

void SysTray::Destroy(/*UINT uID*/)
{
	//myNID.uID = uID;

	if (myActive)
		Shell_NotifyIcon(NIM_DELETE, &myNID);

	myActive = false;

	return;
}

void SysTray::SetText(TCHAR * szTip)
{
	if (!myActive)
		return;

	wcscpy_s(myNID.szTip, 64/*128*/, szTip);
	myNID.uFlags = NIF_TIP;

	Shell_NotifyIcon(NIM_MODIFY, &myNID);

	return;
}

void SysTray::SetIcon(HICON hIcon)
{
	if (!myActive)
		return;

	myNID.hIcon = hIcon;
	myNID.uFlags = NIF_ICON;

	Shell_NotifyIcon(NIM_MODIFY, &myNID);

	return;
}
