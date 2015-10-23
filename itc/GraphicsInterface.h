#pragma once

#include <windows.h>

#include "../common/multimon.h"

#include "GraphicsEngine.h"
#include "node.h"

#include "globaldata.h"
#include "settings.h"

#include <vector>

class GraphicsInterface
{
public:
	GraphicsInterface();
	~GraphicsInterface();

	// DisplayInfo: Display current track info
	void DisplayInfo( wstring wstrText, bool bArtwork = false, wchar_t *wcArt = NULL, int display_time = m_ssSettings.nPopupShowTime, bool bUpdateOnly = false );
	// DisplayStatus: Display temporary status feedback
	void DisplayStatus(wstring wstrText, int display_time = m_ssSettings.nPopupShowTime);

	void DisplayShow();
	void DisplayHide();
	bool IsDisplayVisible();
	void OverrideAlwaysUp(bool override_always_up) { _override_always_up = override_always_up; }

private:
	static VOID CALLBACK DisplayTimerCallback( PVOID lpParameter, BOOLEAN TimerOrWaitFired );
	void DisplayTimerCallback( );

	void Replace(wstring & wstrText, wstring wstrFind, wstring wstrReplace);

	std::vector<GraphicsEngine*> _ges;
	Bitmap *_bmInfo;
	HANDLE _hTimerQueue;
	HANDLE _hDisplayTimer;
	CRITICAL_SECTION _csCreateDisplay;

	bool _override_always_up;
	int _nDisplayType; // 0 - track info, 1 - status feedback
};
