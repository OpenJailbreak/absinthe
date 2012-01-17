#include <wx/wx.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include "AbsintheMainWnd.h"

class Absinthe : public wxApp
{
	virtual bool OnInit();
};

bool Absinthe::OnInit()
{
	AbsintheMainWnd* mainWnd = new AbsintheMainWnd();
	mainWnd->Show(true);
	SetTopWindow(mainWnd);

	return true;
}

IMPLEMENT_APP(Absinthe)
