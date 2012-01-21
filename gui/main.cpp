#include <wx/wx.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include "AbsintheMainWnd.h"
#include "debug.h"

#ifdef WIN32
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#define         ws2s(as)   (std::string((as).mb_str(wxConvUTF8)))

class Absinthe : public wxApp
{
	virtual bool OnInit();
};

#if defined(WIN32)
static bool hasAdminRights() /*{{{*/
{
	HANDLE hAccessToken = NULL;
	TOKEN_GROUPS *ptg;
	DWORD dwInfoBufferSize;
	PSID psidAdmins = NULL;
	bool res = false;
	int bSuccess;

	if ((bSuccess = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hAccessToken))) {
		ptg = (TOKEN_GROUPS *)malloc(1024);
		bSuccess = GetTokenInformation(hAccessToken, TokenGroups, ptg, 1024, &dwInfoBufferSize) ;
		CloseHandle(hAccessToken);
		if (bSuccess) {
			SID_IDENTIFIER_AUTHORITY sia = {SECURITY_NT_AUTHORITY};
			AllocateAndInitializeSid(&sia, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmins);
			if (psidAdmins) {
				unsigned int g;
				for (g = 0; g < ptg->GroupCount; g++) {
					if (EqualSid(psidAdmins, ptg->Groups[g].Sid)) {
						res = true;
						break;
					}
				}
			}
			FreeSid(psidAdmins);
		}
		free(ptg);
	}
	return res;
} /*}}}*/
#endif

bool Absinthe::OnInit()
{
#ifndef WIN32
#if defined(__APPLE__)
        char argv0[1024];
        uint32_t argv0_size = sizeof(argv0);
        _NSGetExecutablePath(argv0, &argv0_size);
#else
	std::string argv_0 = ws2s(argv[0]);
	const char* argv0 = argv_0.c_str();
#endif
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
#else
	TCHAR mfn[512];
	mfn[0] = 0;
	int mfl = GetModuleFileName(NULL, mfn, 512);
	if (mfl > 0) {
		int i;
		for (i = mfl-1; i >= 0; i--) {
			if ((mfn[i] == '/') || (mfn[i] == '\\')) {
				mfn[i] = '\0';
				break;
			}
		}
		if (!SetCurrentDirectory(mfn)) {
			debug("unable to set working directory\n");
		}
	}
#endif

#if defined(__APPLE__) || defined(WIN32)
# if defined(WIN32)
	if (!hasAdminRights()) {
		wxMessageBox(wxT("You must run this app as Administrator."), wxT("Error"), wxOK | wxICON_ERROR);
		return false;
	}
# endif
# if defined(__APPLE__)

# endif
#endif

	AbsintheMainWnd* mainWnd = new AbsintheMainWnd();
	mainWnd->Show(true);
	SetTopWindow(mainWnd);

	return true;
}

IMPLEMENT_APP(Absinthe)
