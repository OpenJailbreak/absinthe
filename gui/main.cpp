#include <wx/wx.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include "AbsintheMainWnd.h"
#include "debug.h"

#define         ws2s(as)   (std::string((as).mb_str(wxConvUTF8)))

class Absinthe : public wxApp
{
	virtual bool OnInit();
};

bool Absinthe::OnInit()
{
	std::string argv_0 = ws2s(argv[0]);
	const char* argv0 = argv_0.c_str();
	char* name = strrchr((char*)argv0, '/');
	if (name) {
		int nlen = strlen(argv0)-strlen(name);
		char path[512];
		memcpy(path, argv0, nlen);
		path[nlen] = 0;
		debug("setting working directory to %s\n", path);
		if (chdir(path) != 0) {
			debug("unable to set working directory\n");
		}
	}

	AbsintheMainWnd* mainWnd = new AbsintheMainWnd();
	mainWnd->Show(true);
	SetTopWindow(mainWnd);

	return true;
}

IMPLEMENT_APP(Absinthe)
