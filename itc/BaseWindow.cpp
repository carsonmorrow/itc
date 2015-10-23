#include "BaseWindow.h"

ATOM s_atomClass = NULL;
ATOM s_atomThisProperty = NULL;


BaseWindow::BaseWindow(void) : _hwnd(NULL)
{
}

BaseWindow::~BaseWindow(void)
{
}

HRESULT BaseWindow::Init()
{
	if (s_atomThisProperty == NULL)
		s_atomThisProperty = GlobalAddAtom(L"BaseWindowThisProperty");

	HWND hWndITC = FindWindow(L"iTunesControl", L"iTunesControl");
	HINSTANCE g_hinst = (HINSTANCE)GetWindowLongPtr(hWndITC, GWLP_HINSTANCE);

	if (s_atomClass == NULL)
	{
		WNDCLASS wc = { 0 };
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpszClassName = TEXT("BaseWindow");
		wc.lpfnWndProc = s_WndProc;
		wc.hInstance = g_hinst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
		s_atomClass = RegisterClass(&wc);
	}

	if (s_atomClass && s_atomThisProperty)
		return S_OK;

	return HRESULT_FROM_WIN32(GetLastError());
}

bool BaseWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pres)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	UNREFERENCED_PARAMETER(pres);
	switch (uMsg)
	{
	case WM_DESTROY:
		OnClose();
		break;
	case WM_NCDESTROY:
		if (_hwnd)
		{

			RemoveProp(_hwnd, (LPTSTR)s_atomThisProperty);
			_hwnd = NULL;
			delete this;
		}
		break;
	}

	return false;

}

LRESULT BaseWindow::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BaseWindow* phost = (BaseWindow*)GetProp(hwnd, (LPTSTR)s_atomThisProperty);
	bool fHandled = false;
	LRESULT lres = 0;
	if (uMsg == WM_CREATE)
	{
		CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
		phost = (BaseWindow*)pcs->lpCreateParams;

		SetProp(hwnd, (LPTSTR)s_atomThisProperty, (HANDLE)phost);
	}

	if (phost)
	{
		fHandled = phost->WndProc(uMsg, wParam, lParam, &lres);
	}

	if (fHandled == false)
		lres = DefWindowProc(hwnd, uMsg, wParam, lParam);

	return lres;
}

