/*
  Copyright © 2005 Alex3D
*/
#include "BBApi.h"
#include "m_alloc.h"


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
	static DWORD clrOutline;
	static DWORD clrOSD;
	static DWORD fontSize;
	static DWORD timeout;
	COsd() {
		m_hFont = NULL;
		m_hWnd = NULL;
		m_strText[0] = 0;
	}
	~COsd() {
		DestroyWindow(m_hWnd);
		UnregisterClass("osd_window", m_hInstance);
		if(m_hFont)
			DeleteObject(m_hFont);
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
	
	void ShowOSD(char* text, int time=0){
		strncpy(m_strText, text, 128);
		ShowWindow(m_hWnd, SW_SHOW);
		InvalidateRect(m_hWnd, NULL, TRUE);
		SetTimer(m_hWnd, 1, time?time:timeout, NULL);
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
		static int msgs[] = { BB_SETTOOLBARLABEL, 0 };
		switch(uMessage)
		{
		case WM_CREATE:
			SendMessage(BBhwnd, BB_REGISTERMESSAGE, (WPARAM)hWnd, (LPARAM)msgs);
			break;

		case WM_DESTROY:
			//PostQuitMessage(0);
			break;

		case WM_PAINT:
			if(!m_strText[0])
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
			ShowOSD((char*)lParam);
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
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		rc.left += 10;
		rc.top += 10;
		SetTextColor(hDC, clrOutline);
		//
		rc.left -= 1;
		rc.top -= 1;
		DrawText(hDC, m_strText, strlen(m_strText), &rc, DT_LEFT);
		rc.left += 2;
		rc.top += 2;
		DrawText(hDC, m_strText, strlen(m_strText), &rc, DT_LEFT);
		rc.left -= 2;
		DrawText(hDC, m_strText, strlen(m_strText), &rc, DT_LEFT);
		rc.left += 2;
		rc.top -= 2;
		DrawText(hDC, m_strText, strlen(m_strText), &rc, DT_LEFT);
		//
		rc.left -= 1;
		rc.top += 1;
		SetTextColor(hDC, clrOSD);
		DrawText(hDC, m_strText, strlen(m_strText), &rc, DT_LEFT);
		EndPaint(m_hWnd, &ps);
	}
	
private:
	HWND			m_hWnd;
	HFONT			m_hFont;
	char			m_strText[128];
	HINSTANCE m_hInstance;
};

DWORD COsd::clrOutline=0x010101;
DWORD COsd::clrOSD=0xffffff;
DWORD COsd::fontSize=75;
DWORD COsd::timeout=3000;


COsd *osd;

int axtoi(char *hexStr) {
  int res = 0;  // integer value of hex string
  for(char* p=hexStr; *p; p++){
     if (*p >= '0' && *p<='9' ) res = res*16 + (*p-'0');
     else
     if (*p >= 'a' && *p<='f' ) res = res*16 + (*p-'a'+10);
     else
     if (*p >= 'A' && *p<='F' ) res = res*16 + (*p-'A'+10);
     else
     break;
  }
  return (res);
}

char* skipwhitespace(char* text){
	char *p=text;
	while(*p==' ' || *p=='\t')p++;
	return p;
}

void LoadConfig()
{
	char rcpath[MAX_PATH]; int i;
	GetModuleFileName(m_hMainInstance, rcpath, sizeof(rcpath));
	for (i=0;;)
	{
		int nLen = strlen(rcpath);
		while (nLen && rcpath[nLen-1] != '\\') nLen--;
		strcpy(rcpath+nLen, "bbosdrc");  if (FileExists(rcpath)) break;
		strcpy(rcpath+nLen, "bbosd.rc"); if (FileExists(rcpath)) break;
		if (2 == ++i)
		{
			return;
		}
		GetBlackboxPath(rcpath, sizeof(rcpath));
	}

	char buffer[1024];
	FILE *fp = FileOpen(rcpath);
	while (ReadNextCommand(fp, buffer, sizeof (buffer))){
		if(!memicmp(buffer,"BBOSD.fontHeight ", 17)){
			COsd::fontSize = atoi(buffer+17);
		}else
		if(!memicmp(buffer,"BBOSD.timeout ", 14)){
			COsd::timeout = atoi(skipwhitespace(buffer+14));
		}
		if(!memicmp(buffer,"BBOSD.clrOSD ", 13)){
			COsd::clrOSD = axtoi(skipwhitespace(buffer+13));
		}else
		if(!memicmp(buffer,"BBOSD.clrOutline ", 17)){
			COsd::clrOutline = axtoi(skipwhitespace(buffer+17));
		}
	}
	FileClose(fp);
}



const char szVersion     [] = "bbOSD 1.0";
const char szAppName     [] = "bbOSD";
const char szInfoVersion [] = "1.0";
const char szInfoAuthor  [] = "alex3d";
const char szInfoRelDate [] = "19/06/2005";
const char szInfoLink    [] = "...";
const char szInfoEmail   [] = "AlexThreeD@users.sourceforge.net";

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
		case 0: return szVersion;
		case 1: return szAppName;
		case 2: return szInfoVersion;
		case 3: return szInfoAuthor;
		case 4: return szInfoRelDate;
		case 5: return szInfoLink;
		case 6: return szInfoEmail;
	}
}


int beginPlugin(HINSTANCE hPluginInstance)
{
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
}


//===========================================================================
