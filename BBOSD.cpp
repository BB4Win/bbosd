/*
BBOSD.cpp
Handles the expected BB4Win Plugin exports

Copyright (c) 2007, Brian Hartvigsen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the BBSOD nor the names of its contributors may be
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
		BBOSD->Destroy();
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