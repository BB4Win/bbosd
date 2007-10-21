#ifndef __BBPLUGIN_H__
#define __BBPLUGIN_H__
#include "BBApi.h"

class CBBPlugin
{
private:
	HINSTANCE m_hInstance;
	static LPSTR szClassName;
	static LPSTR szWindowName;
	static int nMessages[];

protected:
	HWND m_hWindow;
	HWND m_hParentWindow;

public:
	CBBPlugin(HINSTANCE h)
	{
		m_hInstance = h;
		m_hWindow = NULL;
		m_hParentWindow = NULL;
	}

	CBBPlugin(HINSTANCE h, HWND p)
	{
		m_hInstance = h;
		m_hWindow = NULL;
		m_hParentWindow = p;
	}

	~CBBPlugin()
	{
		if (m_hWindow)
			DestroyWindow(m_hWindow);

		UnregisterClass(szClassName, m_hInstance);
	}

	BOOL _RegisterClass()
	{
		WNDCLASSEX wcl = {NULL};
		wcl.cbSize = sizeof(WNDCLASSEX);
		wcl.hInstance = m_hInstance;
		wcl.lpfnWndProc = (WNDPROC)WndProc;
		wcl.style = CS_HREDRAW | CS_VREDRAW;
		wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wcl.lpszClassName = szClassName;
		if(!RegisterClassEx(&wcl))
			return FALSE;

		return TRUE;
	}

	BOOL _CreateWindow(int width = 0, int height = 0)
	{
		m_hWindow = CreateWindowEx(
			WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
			szClassName,
			szWindowName, 
			WS_POPUP, 
			0, 0, width, height,
			NULL,
			NULL, 
			m_hInstance,
			this);

		if (m_hWindow) return TRUE;
		return FALSE;
	}

	BOOL AddToSlit()
	{
		if (!m_hParentWindow)
			return FALSE;

		return (BOOL)SendMessage(m_hParentWindow, SLIT_ADD, NULL, (LPARAM)m_hWindow);
	}

	BOOL RemoveFromSlit()
	{
		if (!m_hParentWindow)
			return FALSE;

		return (BOOL)SendMessage(m_hParentWindow, SLIT_REMOVE, NULL, (LPARAM)m_hWindow);
	}
private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
	{
		static CBBPlugin *pThis = NULL;
		if(uMessage == WM_NCCREATE)
			pThis = (CBBPlugin *)((CREATESTRUCT *)(lParam))->lpCreateParams;
		else if (uMessage == WM_CREATE)
			SendMessage(GetBBWnd(), BB_REGISTERMESSAGE, (WPARAM)hWnd, (LPARAM)nMessages);
		else if (uMessage == WM_DESTROY)
		{
			SendMessage(GetBBWnd(), BB_UNREGISTERMESSAGE, (WPARAM)hWnd, (LPARAM)nMessages);
			return TRUE;
		}
		else if (uMessage == WM_NCDESTROY)
			return TRUE;

		return pThis->WindowProc(hWnd, uMessage, wParam, lParam);
	}

	virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) = 0;
};

#endif