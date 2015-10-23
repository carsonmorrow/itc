#pragma once
#include <windows.h>
#include <tchar.h>
#include "gdiplus_mem.h"

#include "globaldata.h"
#include "settings.h"
#include "../common/multimon.h"
#include <string>

struct inputItem
{
	wstring wstrTitle;
	int nValue;
	int nType; // -1: label (can't be selected), 0: normal (selectable label),
			   //  1: int selector, 2: str array selector, 3: #2 "edit box"
	int nMax;
	int nMin;
	int nID;
	wstring *wstrSelect;
};

struct inputView
{
	wstring wstrTitle;
	inputItem *iiList;
	int nItems;
	int nPos;
	inputView()
	{
		iiList = NULL;
		nItems = 0;
		nPos = 0;
	}
};

class InputWindow
{
protected:

private:
	UINT BitmapIntoLayeredWindow(HWND hWnd, Gdiplus::Bitmap& bmp, POINT &point);
	void DisplayInputInternal();

	int _nAlpha;
	inputView _ivPage;
	HANDLE _hDone;
	HWND _hwnd;

public:
	InputWindow();

	LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int DisplayInput(inputView ivPage);
	void SetHwnd(HWND hwnd);
};

void InitInputEngine();
void ShutdownInputEngine();

LRESULT CALLBACK InputWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//#ifdef _DEBUG
//#define _DBGLOG(msg) (void)(OutputDebugString(msg))
//#else
#define _DBGLOG(msg)
//#endif