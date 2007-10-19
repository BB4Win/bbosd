/*
  Copyright © 2005 Alex3D
*/
#include "BBApi.h"
#include <gdiplus.h>
//#include "m_alloc.h"

//===========================================================================
void LoadConfig();

//===========================================================================
HWND BBhwnd;
HINSTANCE m_hMainInstance;
bool usingNT;
BOOL (WINAPI*pSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
HINSTANCE hUser32;

//===========================================================================

	
class COsd
{
public:
	static COLORREF clrOutline;
	static COLORREF clrOSD;
	static DWORD	edgePadding;
	static DWORD    fontSize;
	static DWORD    timeout;
	static bool     bShowLabel;
	static ULONG_PTR ulGdiplusToken;
	static char		sPosition[MAX_LINE_LENGTH];

	COsd() {
		m_hFont = NULL;
		m_hWnd = NULL;
		m_strText = NULL;
	}

	~COsd() {
		DestroyWindow(m_hWnd);
		UnregisterClass("osd_window", m_hInstance);
		if(m_hFont)
			DeleteObject(m_hFont);
		if (m_strText)
			delete[] m_strText;
	}

	BOOL Initialize(HINSTANCE hInstance)
	{
		m_hInstance = hInstance;
		HDC hDC = GetDC(NULL);
		int nHeight = fontSize*GetDeviceCaps(hDC, LOGPIXELSY)/100;
		ReleaseDC(NULL, hDC);
		
		m_hFont = CreateFont(nHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, "Verdana");
		if(!m_hFont)
			return FALSE;

		char* class_name = "osd_window";
		WNDCLASSEX wcl = {NULL};
		wcl.cbSize = sizeof(WNDCLASSEX);
		wcl.hInstance = hInstance;
		wcl.lpfnWndProc = (WNDPROC)WndProc;
		wcl.style = CS_HREDRAW | CS_VREDRAW;
		wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wcl.lpszClassName = class_name;
		if(!RegisterClassEx(&wcl))
			return FALSE;

		HWND hWndDesktop = GetDesktopWindow();

		RECT rcClient;
		if(!GetClientRect(hWndDesktop, &rcClient))
			return FALSE;

		m_hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
			 class_name, NULL, 
			WS_VISIBLE | WS_POPUP, 
			0, 0, 800, nHeight+30, 
			hWndDesktop, NULL, 
			hInstance, this);

		if (pSetLayeredWindowAttributes)
				pSetLayeredWindowAttributes(m_hWnd, 0, 0, LWA_COLORKEY);

		if(!m_hWnd)
			return FALSE;

		return TRUE;
	}

/*********************************************************************
* PositionWindow                                                     *
*                                                                    *
* Determines nessacary height/width of client window.  Resizes and   *
* positions window accordingly prior to drawing.                     *
* Uses GDI+ as it's the only way I found to do this crap :P          *
*********************************************************************/
	void PositionWindow()
	{
		HDC hDC = CreateCompatibleDC(NULL);
		Gdiplus::Graphics g(hDC);
		Gdiplus::RectF boundingRect;
		Gdiplus::RectF layoutRect;
		Gdiplus::Font font(hDC, m_hFont);
		int length = lstrlen(m_strText);

		WCHAR *wstring = new WCHAR[length + 1];

		Gdiplus::REAL size = font.GetHeight(&g);

		MultiByteToWideChar(CP_ACP, 0, m_strText, -1, wstring, length + 1);

		g.MeasureString(wstring, length, &font, layoutRect, &boundingRect);
		delete[] wstring;

		int x = COsd::edgePadding;
		int y = COsd::edgePadding;

// Get lots of warnings for REAL -> int conversions..
#pragma warning(disable: 4244)
		if (IsInString(COsd::sPosition, "middle"))
			y = (GetSystemMetrics(SM_CYSCREEN) / 2) - (boundingRect.Height / 2);
		else if (IsInString(COsd::sPosition, "bottom"))
			y = GetSystemMetrics(SM_CYSCREEN) - boundingRect.Height - COsd::edgePadding;

		if (IsInString(COsd::sPosition, "center"))
			x = (GetSystemMetrics(SM_CXSCREEN) / 2) - (boundingRect.Width / 2);
		else if (IsInString(COsd::sPosition, "right"))
			x = GetSystemMetrics(SM_CXSCREEN) - boundingRect.Width - COsd::edgePadding;

		SetWindowPos(m_hWnd, NULL, x, y, boundingRect.Width, boundingRect.Height, SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW);
		InvalidateRect(m_hWnd, NULL, TRUE);
#pragma warning(default: 4244)

		DeleteDC(hDC);
	}

/********************************************************************
* ShowOSD                                                           *
*                                                                   *
* Takes broam text or toolbar label and saves it to be painted when *
* we invalidate the rect in PositionWindow ;)                       *
********************************************************************/
	void ShowOSD(char* text, int time=0){
		if (m_strText != NULL)
			delete[] m_strText;

		m_strText = new char[lstrlen(text) + 1];
		strcpy(m_strText, text);

		PositionWindow();
		SetTimer(m_hWnd, 1, time?time:timeout, NULL);
	}

	void InterpretBroam(char *b)
	{
		char *broam = new char[strlen(b) + 1];
		strcpy(broam,b);

		if (!_strnicmp(broam, "@BBOSD ", 7))
		{
			size_t len = lstrlen(broam) + 1;
			char *drop = new char[len];
			char *osd = new char[len];
			char *timeout = new char[len];
			char *tokens[2] = { drop, osd };

			drop[0] = osd[0] = timeout[0] = 0;
			int toks = BBTokenize(broam, tokens, 2, timeout);

			ShowOSD(osd, atoi(timeout));
			delete[] drop;
			delete[] osd;
			delete[] timeout;
		}
		delete[] broam;
	}

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
	{
		static COsd *pThis = NULL;
		if(uMessage == WM_CREATE)
			pThis = (COsd *)((CREATESTRUCT *)(lParam))->lpCreateParams;

		return pThis->WindowProc(hWnd, uMessage, wParam, lParam);
	}

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
	{
		static int msgs[] = { BB_SETTOOLBARLABEL, BB_BROADCAST, BB_RECONFIGURE, 0 };
		switch(uMessage)
		{
		case WM_CREATE:
			SendMessage(BBhwnd, BB_REGISTERMESSAGE, (WPARAM)hWnd, (LPARAM)msgs);
			break;

		case WM_DESTROY:
			//PostQuitMessage(0);
			break;

		case WM_PAINT:
			if(m_strText == NULL)
				return DefWindowProc(hWnd, uMessage, wParam, lParam);

			OnPaint();
			break;

		case WM_TIMER:
			if(wParam == 1)
			{
				ShowWindow(hWnd, SW_HIDE);
				KillTimer(hWnd, 1);
			}
			break;

		case BB_SETTOOLBARLABEL:
			if (bShowLabel)
				ShowOSD((char*)lParam);
			break;

		case BB_BROADCAST:
			InterpretBroam((char *)lParam);
			break;

		case BB_RECONFIGURE:
			::LoadConfig();
			break;

		default:
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}

		return 0;
	}

	void OnPaint()
	{
		PAINTSTRUCT ps = {NULL};
		HDC hDC = BeginPaint(m_hWnd, &ps);
		SetBkMode(hDC, TRANSPARENT);
		SelectObject(hDC, m_hFont);

		int length = lstrlen(m_strText);

		RECT rc;
		GetClientRect(m_hWnd, &rc);

		SetTextColor(hDC, clrOutline);
		//
		rc.left -= 1;
		rc.top -= 1;
		DrawText(hDC, m_strText, length, &rc, DT_LEFT);
		rc.left += 2;
		rc.top += 2;
		DrawText(hDC, m_strText, length, &rc, DT_LEFT);
		rc.left -= 2;
		DrawText(hDC, m_strText, length, &rc, DT_LEFT);
		rc.left += 2;
		rc.top -= 2;
		DrawText(hDC, m_strText, length, &rc, DT_LEFT);
		//
		rc.left -= 1;
		rc.top += 1;
		SetTextColor(hDC, clrOSD);
		DrawText(hDC, m_strText, length, &rc, DT_LEFT);

		EndPaint(m_hWnd, &ps);
	}
	
private:
	HWND			m_hWnd;
	HFONT			m_hFont;
	char*			m_strText;
	HINSTANCE		m_hInstance;
};

COLORREF COsd::clrOutline=0x010101;
COLORREF COsd::clrOSD=0xffffff;
DWORD    COsd::fontSize=75;
DWORD	 COsd::edgePadding=10;
DWORD    COsd::timeout=3000;
bool     COsd::bShowLabel=true;
ULONG_PTR COsd::ulGdiplusToken = NULL;
char	 COsd::sPosition[]="topleft";


COsd *osd;

void LoadConfig()
{
	char rcpath[MAX_PATH]; int i;
	GetModuleFileName(m_hMainInstance, rcpath, sizeof(rcpath));
	for (i=0;;)
	{
		int nLen = lstrlen(rcpath);
		while (nLen && rcpath[nLen-1] != '\\') nLen--;
		strcpy(rcpath+nLen, "bbosdrc");  if (FileExists(rcpath)) break;
		strcpy(rcpath+nLen, "bbosd.rc"); if (FileExists(rcpath)) break;
		if (2 == ++i)
		{
			return;
		}
		GetBlackboxPath(rcpath, sizeof(rcpath));
	}

	COsd::edgePadding = ReadInt(rcpath, "BBOSD.edgePadding:", COsd::edgePadding);
	COsd::fontSize = ReadInt(rcpath, "BBOSD.fontHeight:", COsd::fontSize);
	COsd::timeout = ReadInt(rcpath, "BBOSD.timeout:", COsd::timeout);
	COsd::clrOSD = ReadColor(rcpath, "BBOSD.clrOSD:", "#ffffff");
	COsd::clrOutline = ReadColor(rcpath, "BBOSD.clrOutline:", "#010101");
	COsd::bShowLabel = ReadBool(rcpath, "BBOSD.showLabel:", COsd::bShowLabel);
	strcpy(COsd::sPosition, ReadString(rcpath, "BBOSD.position:", COsd::sPosition));
}



const char szVersion     [] = "bbOSD 1.4.4";
const char szAppName     [] = "bbOSD";
const char szInfoVersion [] = "1.4.4";
const char szInfoAuthor  [] = "alex3d/Tres`ni";
const char szInfoRelDate [] = "13 Apr 2007";
const char szInfoEmail   [] = "AlexThreeD@users.sourceforge.net/tresni@crackmonkey.us";

//===========================================================================
extern "C"
{
	DLL_EXPORT int beginPlugin(HINSTANCE hMainInstance);
	DLL_EXPORT void endPlugin(HINSTANCE hMainInstance);
	DLL_EXPORT LPCSTR pluginInfo(int field);
}

LPCSTR pluginInfo(int field)
{
	switch (field)
	{
		default:
			return szVersion;
		case PLUGIN_NAME:
			return szAppName;
		case PLUGIN_VERSION:
			return szInfoVersion;
		case PLUGIN_AUTHOR:
			return szInfoAuthor;
		case PLUGIN_RELEASEDATE:
			return szInfoRelDate;
		case PLUGIN_EMAIL:
			return szInfoEmail;
#if _DEBUG
		case PLUGIN_BROAMS:
			return "@BBOSD abcdefghijklmnopqrstuvwxyz";
#endif
	}
}


int beginPlugin(HINSTANCE hPluginInstance)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&COsd::ulGdiplusToken, &gdiplusStartupInput, NULL);

	pSetLayeredWindowAttributes=0;
	hUser32=LoadLibraryA("user32.dll");
	if (hUser32) {
		pSetLayeredWindowAttributes=(BOOL(WINAPI*)(HWND, COLORREF, BYTE, DWORD))GetProcAddress(hUser32, "SetLayeredWindowAttributes");
		if (!pSetLayeredWindowAttributes) {
			FreeLibrary(hUser32);
			hUser32=0;
		}
	}
	if(!pSetLayeredWindowAttributes){
		MessageBox(NULL, "OS version not supported (Windows2000+ required)", szVersion, MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
		return 1;
	}
	
	if (BBhwnd)
	{
		MessageBox(NULL, "Dont load me twice!", szAppName, MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
		return 1;
	}

	BBhwnd = GetBBWnd();

	m_hMainInstance = hPluginInstance;

	LoadConfig();
	osd = new COsd();
	if(!osd->Initialize(hPluginInstance)){
		return 1;
	}
	return 0;
}


void endPlugin(HINSTANCE hPluginInstance)
{
	delete osd;
	Gdiplus::GdiplusShutdown(COsd::ulGdiplusToken);
}


//===========================================================================
