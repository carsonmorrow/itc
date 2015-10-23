#pragma once

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502
#define _WIN32_IE 0x0600

//#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
using namespace Gdiplus;

class BaseWindow
{
protected: 
    HWND _hwnd;
    virtual bool WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pres);

    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static HRESULT Init();
    virtual ~BaseWindow(void);
    BaseWindow(void);

    virtual void OnClose() { };
public:

    HWND GetWindow() { return _hwnd; }
};

extern ATOM s_atomClass;
extern ATOM s_atomThisProperty;

