#include "multimon.h"

CMonitors::CMonitors()
{
	this->Initialize();
}

CMonitors::~CMonitors()
{
}

int CMonitors::GetCount()
{
	return _bValid ? (int)_vecInfo.size() : 0;
}

bool CMonitors::Initialize()
{
	HDC hdcScreen = NULL;

	_vecInfo.clear();
	_bValid = true;
	hdcScreen = GetDC(NULL);

	if (!EnumDisplayMonitors(hdcScreen, NULL, MonitorEnumProc, (LPARAM)this))
		_bValid = false;

	if (!_bValid)
	{
		_vecInfo.clear();
	}

	ReleaseDC(NULL, hdcScreen);

	return _bValid;
}

bool CMonitors::GetMonitorRect(UINT uiMonitor, RECT &rcMonitor)
{
	if (uiMonitor <= _vecInfo.size() && _bValid)
	{
		rcMonitor = _vecInfo.at(uiMonitor - 1).rcWork;
		return true;
	}
	else
		return false; //we don't have that many monitors
}

/* static */ BOOL CALLBACK CMonitors::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	CMonitors *self = (CMonitors *)dwData;

	return self->MonitorEnumProc(hMonitor, hdcMonitor, lprcMonitor);
}

BOOL CMonitors::MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT)
{
	MONITORINFOEX info;

	info.cbSize = sizeof(MONITORINFOEX);

	if (!GetMonitorInfo(hMonitor, &info))
	{
		_bValid = false;
		return FALSE;
	}

	_vecInfo.push_back(info);

	return TRUE;
}