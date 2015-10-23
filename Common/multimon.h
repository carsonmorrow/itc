#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <vector>
using namespace std;

class CMonitors {
public:
	CMonitors();
	~CMonitors();
	bool Initialize();
	int GetCount();
	bool GetMonitorRect(UINT uiMonitor, RECT &rcMonitor);

private:
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
	BOOL MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor);
	vector<MONITORINFOEX> _vecInfo;
	bool _bValid;
};