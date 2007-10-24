/*
BBPlugin.h
Base OO Implementation of a BB4Win plugin

Copyright (c) 2007, Brian Hartvigsen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the BBPlugin nor the names of its contributors may be
	  used to endorse or promote products derived from this software without
	  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

	void _DestroyWindow()
	{
		if (m_hWindow)
			DestroyWindow(m_hWindow);

		UnregisterClass(szClassName, m_hInstance);
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
			SendMessage(GetBBWnd(), BB_UNREGISTERMESSAGE, (WPARAM)hWnd, (LPARAM)nMessages);
		else if (uMessage == WM_NCDESTROY)
			pThis = NULL;

		if (pThis != NULL)
			return pThis->WindowProc(hWnd, uMessage, wParam, lParam);
		else
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}

	virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) = 0;
};

#endif