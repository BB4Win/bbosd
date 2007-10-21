#include "BBPlugin.h"

#define POSITION_TOP	0x0001
#define POSITION_MIDDLE	0x0002
#define POSITION_BOTTOM	0x0004
#define POSITION_LEFT	0x0010
#define POSITION_CENTER	0x0020
#define POSITION_RIGHT	0x0040

class CBBOSD : public CBBPlugin {
public:
	CBBOSD(HINSTANCE h) : CBBPlugin(h)
	{
		m_hInstance = h;
		m_hMemDC = NULL;
		m_hMemBitmap = NULL;
		m_hFont = NULL;
		m_szOsdText = NULL;
	}

	~CBBOSD()
	{
		DeleteObject(SelectObject(m_hMemDC, m_hFont));
		DeleteObject(SelectObject(m_hMemDC, m_hMemBitmap));
		ReleaseDC(m_hWindow,m_hMemDC);
		
		delete[] m_szOsdText;
	}

	int Initialize()
	{
		_RegisterClass();
		_CreateWindow(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

		BOOL (WINAPI*pSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD) = NULL;
		HMODULE hUser32 = LoadLibraryA("user32.dll");
		if (hUser32) {
			pSetLayeredWindowAttributes=(BOOL(WINAPI*)(HWND, COLORREF, BYTE, DWORD))GetProcAddress(hUser32, "SetLayeredWindowAttributes");
			if (!pSetLayeredWindowAttributes) {
				FreeLibrary(hUser32);
				hUser32=NULL;
			}
		}
		if(!pSetLayeredWindowAttributes){
			MessageBox(NULL, "OS version not supported (Windows2000+ required)", "BBOSD", MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
			return TRUE;
		}

		if (pSetLayeredWindowAttributes)
			pSetLayeredWindowAttributes(m_hWindow, 0, 0, LWA_COLORKEY);

		if (ReadSettings())
			return FALSE;

		return TRUE;
	}

	bool ReadSettings()
	{
		char rcpath[MAX_PATH]; int i;
		GetModuleFileName(m_hInstance, rcpath, sizeof(rcpath));
		for (i=0;;)
		{
			int nLen = lstrlen(rcpath);
			while (nLen && rcpath[nLen-1] != '\\') nLen--;
			strcpy(rcpath+nLen, "bbosdrc");  if (FileExists(rcpath)) break;
			strcpy(rcpath+nLen, "bbosd.rc"); if (FileExists(rcpath)) break;
			if (2 == ++i)
			{
				strncpy(rcpath, extensionsrcPath(), MAX_PATH);
			}
			GetBlackboxPath(rcpath, sizeof(rcpath));
		}

		char setting[MAX_LINE_LENGTH];
		ZeroMemory(setting, MAX_LINE_LENGTH);

		m_bShowToolbarLabel = ReadBool(rcpath, "bbOSD.ShowLabel:", true);
		bool bVoodooMath = ReadBool(rcpath, "bbOSD.VoodooMath:", false);
		m_nEdgePadding = ReadInt(rcpath, "bbOSD.EdgePadding:", 10);
		m_nTimeout = ReadInt(rcpath, "bbOSD.Timeout:", 30000);
		m_cFontColor = ReadColor(rcpath, "bbosd.ClrOSD:", "#FFFFFF");
		m_cBorderColor = ReadColor(rcpath, "bbOSD.ClrOutline:", "#010101");

		strncpy(setting, ReadString(rcpath, "bbOSD.Position:", ""), MAX_LINE_LENGTH);
		if (IsInString(setting, "Middle"))
			m_dPosition = POSITION_MIDDLE;
		else if(IsInString(setting, "Bottom"))
			m_dPosition = POSITION_BOTTOM;
		else
			m_dPosition = POSITION_TOP;

		if (IsInString(setting, "Center"))
			m_dPosition |= POSITION_CENTER;
		else if (IsInString(setting, "Right"))
			m_dPosition |= POSITION_RIGHT;
		else
			m_dPosition |= POSITION_LEFT;

		InitializeImages();

		strncpy(setting, ReadString(rcpath, "bbOSD.FontFace:", "Verdana"), MAX_LINE_LENGTH);

		int size = ReadInt(rcpath, "bbOSD.FontHeight:", 75);
		size = (bVoodooMath ? 1 : -1) * MulDiv(size, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), bVoodooMath ? 100 : 72);

		HFONT temp = CreateFont(size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, setting);
		if (!temp && !m_hFont)
			return false;
		else
		{
			if (temp)
			{
				if (m_hFont)
					// Put the old font back and delete the new one
					DeleteObject(SelectObject(m_hMemDC, m_hFont));

				m_hFont = temp;
			}
		}

		m_hFont = (HFONT)SelectObject(m_hMemDC, m_hFont);
		return true;
	}

	void InitializeImages()
	{
		m_hMemDC = CreateCompatibleDC(NULL);
		m_hMemBitmap = CreateCompatibleBitmap(m_hMemDC, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		m_hMemBitmap = (HBITMAP)SelectObject(m_hMemDC, m_hMemBitmap);
	}

	void OnDraw()
	{
		int height = GetSystemMetrics(SM_CYSCREEN);
		int width = GetSystemMetrics(SM_CXSCREEN);

		RECT rc = {0, 0, width, height};

		HBRUSH black = CreateSolidBrush(RGB(0,0,0));
		HBRUSH old = (HBRUSH)SelectObject(m_hMemDC, black);
		FillRect(m_hMemDC, &rc, black);

		SelectObject(m_hMemDC, old);
		DeleteObject(black);

		DrawText(m_hMemDC, m_szOsdText, -1, &rc, DT_CALCRECT);

		int yOffset = m_nEdgePadding, xOffset = m_nEdgePadding;
		if (m_dPosition & POSITION_MIDDLE)
			yOffset =  (height / 2) - (rc.bottom / 2);
		else if (m_dPosition & POSITION_BOTTOM)
			yOffset = height - rc.bottom - m_nEdgePadding;

		if (m_dPosition & POSITION_CENTER)
			xOffset = (width / 2) - (rc.right / 2);
		else if (m_dPosition & POSITION_RIGHT)
			xOffset = width - rc.right - m_nEdgePadding;

		SetWindowPos(m_hWindow, NULL, xOffset, yOffset, rc.right, rc.bottom, SWP_NOSENDCHANGING | SWP_NOACTIVATE | SWP_NOZORDER);
		SetBkMode(m_hMemDC, TRANSPARENT);

		rc.left -= 1;
		rc.top -= 1;
		SetTextColor(m_hMemDC, m_cBorderColor);
		DrawText(m_hMemDC, m_szOsdText, -1, &rc, DT_LEFT);

		rc.left += 2;
		rc.top += 2;
		DrawText(m_hMemDC, m_szOsdText, -1, &rc, DT_LEFT);

		rc.left -= 2;
		DrawText(m_hMemDC, m_szOsdText, -1, &rc, DT_LEFT);

		rc.left += 2;
		rc.top -= 2;
		DrawText(m_hMemDC, m_szOsdText, -1, &rc, DT_LEFT);

		rc.left -= 1;
		rc.top += 1;
		SetTextColor(m_hMemDC, m_cFontColor);
		DrawText(m_hMemDC, m_szOsdText, -1, &rc, DT_LEFT);
	}

	void OnPaint()
	{
		PAINTSTRUCT ps;
		BeginPaint(m_hWindow, &ps);

		BitBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, m_hMemDC, 0, 0, SRCCOPY);

		EndPaint(m_hWindow, &ps);
	}

	void OnBroam(LPCSTR broam)
	{
		char *b = new char[strlen(broam) + 1];
		strcpy(b,broam);

		if (!_strnicmp(b, "@BBOSD ", 7))
		{
			size_t len = lstrlen(b) + 1;
			char *drop = new char[len];
			char *osd = new char[len];
			char *timeout = new char[len];
			char *tokens[2] = { drop, osd };

			drop[0] = osd[0] = timeout[0] = 0;
			int toks = BBTokenize(b, tokens, 2, timeout);

			ShowOSD(osd, atoi(timeout));
			delete[] drop;
			delete[] osd;
			delete[] timeout;
		}
		delete[] b;
	}

	void ShowOSD(LPCSTR message, int timeout)
	{
		if (!m_szOsdText || _stricmp(message, m_szOsdText))
		{
			if (m_szOsdText)
				delete[] m_szOsdText;

			m_szOsdText = new char[lstrlen(message) + 1];
			strcpy(m_szOsdText, message);

			OnDraw();
		}

		InvalidateRect(m_hWindow, NULL, TRUE);
		ShowWindow(m_hWindow, SW_SHOW);
		SetTimer(m_hWindow, 1, timeout ? timeout : m_nTimeout, NULL);
	}

	virtual	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
	{
		switch(uMessage)
		{
		case WM_CLOSE: break;

		case WM_PAINT:
			OnPaint();
			break;

		case WM_TIMER:
			if(wParam == 1)
			{
				ShowWindow(m_hWindow, SW_HIDE);
				KillTimer(m_hWindow, 1);
			}
			break;

		case BB_SETTOOLBARLABEL:
			if (m_bShowToolbarLabel)
				ShowOSD((char*)lParam, 0);
			break;

		case BB_BROADCAST:
			OnBroam((char *)lParam);
			break;

		case BB_RECONFIGURE:
			ReadSettings();
			break;

		default:
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}

		return FALSE;
	}

private:

	// Windows
	HINSTANCE m_hInstance;

	// Gdi
	HDC m_hMemDC;
	HBITMAP m_hMemBitmap;
	HFONT m_hFont;

	// RC Settings
	bool m_bShowToolbarLabel;
	int m_nEdgePadding;
	int m_nTimeout;
	DWORD m_dPosition;
	COLORREF m_cFontColor;
	COLORREF m_cBorderColor;

	// Other
	LPSTR m_szOsdText;
};