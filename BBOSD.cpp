#include "BBOSD.h"

CBBOSD *BBOSD;

const char szVersion     [] = "bbOSD 2.0.0";
const char szAppName     [] = "bbOSD";
const char szInfoVersion [] = "2.0.0";
const char szInfoAuthor  [] = "Tres`ni";
const char szInfoRelDate [] = "20 Oct 2007";
const char szInfoEmail   [] = "tresni@crackmonkey.us";

LPSTR CBBPlugin::szClassName = "BBOSD_Class";
LPSTR CBBPlugin::szWindowName = "BBOSD_Window";
int CBBPlugin::nMessages[] = { BB_SETTOOLBARLABEL, BB_BROADCAST, BB_RECONFIGURE, 0 };

extern "C" {
	DLL_EXPORT int beginPlugin(HINSTANCE hInstance)
	{
		BBOSD = new CBBOSD(hInstance);
		return BBOSD->Initialize();
	}

	DLL_EXPORT void endPlugin(HINSTANCE hInstance)
	{
		delete BBOSD;
	}

	DLL_EXPORT LPCSTR pluginInfo(int field)
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
}