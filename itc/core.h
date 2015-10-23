#ifndef __CORE_H__INCLUDED
#define __CORE_H__INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/timeb.h>

#include "resource.h"
#include "settings.h"
#include "globaldata.h"

class SysTray
{
public:
	SysTray();
	~SysTray();
	void Create(HWND hWndOwner, TCHAR * szTip, HICON hIcon, UINT uID, UINT uCBMsg);
	void SetText(TCHAR * szTip);
	void SysTray::SetIcon(HICON hIcon);
	void ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags);
	void Destroy(/*UINT uID*/);
	inline bool IsActive() { return myActive; }

private:
	NOTIFYICONDATA myNID;
	bool myActive;
};

void PowerControl( int nType );

void SetState(bool bConnected);

#endif