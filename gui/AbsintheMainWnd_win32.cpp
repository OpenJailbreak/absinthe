#include <windows.h>
#include <commctrl.h>
#include <version.h>
#include "AbsintheMainWnd.h"

#include <stdio.h>

static AbsintheMainWnd* self;

static HWND panel = NULL;
static HWND lbInfo = NULL;
static HWND lbCredits = NULL;
static HWND lbPaypal = NULL;
static HWND lbGP = NULL;

int AbsintheMainWnd::msgBox(const char* message, const char* caption, int style)
{
	int res = 0;

	// get message type
	int mtype = 0;
	if (style & mb_ICON_INFO) {
		mtype = MB_ICONINFORMATION;
	} else if (style & mb_ICON_WARNING) {
		mtype = MB_ICONWARNING;
	} else if (style & mb_ICON_QUESTION) {
		mtype = MB_ICONQUESTION;
	} else if (style & mb_ICON_ERROR) {
		mtype = MB_ICONERROR;
	}

	// get button type(s)
	int btype = MB_OK;
	if (style & mb_OK) {
		btype = MB_OK;
	} else if (style & mb_CANCEL) {
		btype = MB_OK;
	} else if (style & mb_OK_CANCEL) {
		btype = MB_OKCANCEL;
	} else if (style & mb_YES_NO) {
		btype = MB_YESNO;
	}

	HWND hnd;
	if (mainwnd == NULL) {
		hnd = GetActiveWindow();
	} else {
		hnd = this->mainwnd;	
	}

	int answer = MessageBox(hnd, message, caption, btype | mtype);

	switch (answer) {
	case IDOK:
		return mb_OK;
	case IDCANCEL:
		return mb_CANCEL;
	case IDYES:
		return mb_YES;
	case IDNO:
		return mb_NO;
	default:
		return -1;
	}
}

void AbsintheMainWnd::setButtonEnabled(int enabled)
{
	EnableWindow(this->btnStart, enabled);
}

void AbsintheMainWnd::setStatusText(const char* text)
{
	int mblen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, NULL, 0);
	if (mblen == 0) {
		SetWindowTextA(this->lbStatus, text);
		return;
	}
	WCHAR* mbstr = (WCHAR*)malloc(mblen*sizeof(WCHAR*));
	if (!mbstr) {
		SetWindowTextA(this->lbStatus, text);
		return;
	}
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, mbstr, mblen) == 0) {
		SetWindowTextA(this->lbStatus, text);
		free(mbstr);
		return;
	}
	SetWindowTextW(this->lbStatus, mbstr);
	free(mbstr);
}

void AbsintheMainWnd::setProgress(int percentage)
{
	SendMessage(this->progressBar, PBM_SETPOS, (WPARAM)percentage, 0);
}

void AbsintheMainWnd::handleStartClicked(void* data)
{
	SetWindowPos(this->mainwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	this->setButtonEnabled(0);
	this->setProgress(0);
	this->worker->processStart();
}

bool AbsintheMainWnd::onClose(void* data)
{
	if (this->closeBlocked) {
		return 1;
	}
	return 0;
}

extern "C" {

static LRESULT CALLBACK PanelWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		case WM_COMMAND:
			if ((HWND)lParam == lbPaypal) {
				SetWindowPos(self->mainwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				ShellExecute(NULL, "open", PAYPAL_LINK_URL, NULL, NULL, SW_SHOWNORMAL);
			} else if ((HWND)lParam == lbGP) {
				SetWindowPos(self->mainwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				ShellExecute(NULL, "open", GP_LINK_URL, NULL, NULL, SW_SHOWNORMAL);
			}
			break;
		case WM_DRAWITEM:
			{
			LPDRAWITEMSTRUCT lpDrawItem;
			HBRUSH hBrush;
			int state;

			lpDrawItem = (LPDRAWITEMSTRUCT) lParam;
			SetTextAlign(lpDrawItem->hDC, TA_CENTER | TA_TOP);
			SetTextColor(lpDrawItem->hDC, RGB(0, 0, 255));
			SetBkMode(lpDrawItem->hDC,TRANSPARENT);

			char txt[256];
			GetWindowText(lpDrawItem->hwndItem, txt, 256);
			TextOut(lpDrawItem->hDC, 75, 2, txt, strlen(txt));

			return 1;
			}
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		case WM_COMMAND:
			if ((HWND)lParam == self->btnStart) {
				self->handleStartClicked(NULL);
			}
			break;
		case WM_CLOSE:
			if (hWnd == self->mainwnd) {
				if (self->onClose(NULL) == 0) {
					DestroyWindow(self->mainwnd);
				}
			}
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

}

static char* fromUTF8(const char* text)
{
	WCHAR* mbstr = NULL;
	int mblen = 0;
	char* str = NULL;
	int slen = 0;

	// get required length
	mblen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, NULL, 0);
	if (mblen == 0) {
		goto leave_error;
	}
	mbstr = (WCHAR*)malloc(mblen * sizeof(WCHAR));
	if (!mbstr) {
		goto leave_error;
	}

	// perform conversion to WCHAR*
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, mbstr, mblen);
	
	// get required length
	slen = WideCharToMultiByte(CP_ACP, 0, mbstr, mblen, NULL, 0, NULL, NULL);
	if (slen == 0) {
		goto leave_error;
	}
	str = (char*)malloc(slen);
	if (!str) {
		goto leave_error;
	}
	
	// convert to OEMCP
	WideCharToMultiByte(CP_ACP, 0, mbstr, mblen, str, slen, NULL, NULL);
	free(mbstr);

	return str;
	
leave_error:
	if (mbstr) {
		free(mbstr);
	}
	if (str) {
		free(str);
	}
	return strdup("text");
}

AbsintheMainWnd::AbsintheMainWnd(int* pargc, char*** pargv)
{
	HINSTANCE hInst = (HINSTANCE)*pargc;
	WNDCLASS WinClass;
	WNDCLASS PanelClass;
	HFONT hFont1;
	int xOffset = 0;
	int yOffset = 0;
	int Width = 0;
	int Height = 0;	

	self = this;

	memset(&WinClass, 0, sizeof(WNDCLASS));
	
	WinClass.style = CS_CLASSDC | CS_PARENTDC;
	WinClass.lpfnWndProc = (WNDPROC)WindowProc;
	WinClass.cbClsExtra = 0;
	WinClass.cbWndExtra = 0;
	WinClass.hInstance = hInst;
	WinClass.hIcon = LoadIcon(hInst, "AppIcon");
	WinClass.hCursor = LoadCursor(0 , IDC_ARROW);
	WinClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	WinClass.lpszMenuName = NULL;
	WinClass.lpszClassName = WND_TITLE "_CLASS";

	PanelClass.style = CS_CLASSDC | CS_PARENTDC;
	PanelClass.lpfnWndProc = (WNDPROC)PanelWindowProc;
	PanelClass.cbClsExtra = 0;
	PanelClass.cbWndExtra = 0;
	PanelClass.hInstance = hInst;
	PanelClass.hIcon = 0;
	PanelClass.hCursor = LoadCursor(0 , IDC_ARROW);
	PanelClass.hbrBackground = 0;
	PanelClass.lpszMenuName = NULL;
	PanelClass.lpszClassName = "Panel";

	if (!RegisterClass(&WinClass)) {
		msgBox("Error registering window class! This should not happen!", "Error", MB_OK | MB_ICONSTOP);
		return;
	}

	if (!RegisterClass(&PanelClass)) {
		msgBox("Error registering panel class! This should not happen!", "Error", MB_OK | MB_ICONSTOP);
		return;
	}

	xOffset = GetSystemMetrics(SM_CXSCREEN);
	yOffset = GetSystemMetrics(SM_CYSCREEN);

	xOffset = (xOffset - WND_WIDTH) / 2;
	yOffset = (yOffset - WND_HEIGHT) / 2;

	InitCommonControls();

	mainwnd = CreateWindowEx(WS_EX_TOPMOST, WinClass.lpszClassName, WND_TITLE, WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, xOffset, yOffset, WND_WIDTH, WND_HEIGHT, 0, 0, hInst, NULL);
	if (mainwnd == NULL) {
		return;
	}

	char* tmpstr;
	tmpstr = fromUTF8(INFO_LABEL_TEXT);
	lbInfo = CreateWindowEx(0, "Static", tmpstr, WS_VISIBLE | WS_CHILD | SS_NOPREFIX, 10, 10, WND_WIDTH-20, 70, mainwnd, (HMENU)1, hInst, NULL);
	free(tmpstr);

	lbStatus = CreateWindowEx(0, "Static", "", WS_VISIBLE | WS_CHILD | SS_NOPREFIX, 10, 86, WND_WIDTH-20, 82, mainwnd, (HMENU)2, hInst, NULL);

	progressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 38, 177, 300, 17, mainwnd, (HMENU)3, hInst, NULL);
	SendMessage(progressBar, PBM_SETRANGE, 0, MAKELPARAM(0,100));
	SendMessage(progressBar, PBM_SETPOS, (WPARAM)0, 0);

	btnStart = CreateWindowEx(0, "Button", "Jailbreak", WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED, 358, 174, 75, 23, mainwnd, (HMENU)4, hInst, NULL);

	tmpstr = fromUTF8(CREDITS_LABEL_TEXT);
	lbCredits = CreateWindowEx(0, "Static", tmpstr, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOPREFIX, 10, 210, WND_WIDTH-20, 80, mainwnd, (HMENU)5, hInst, NULL);
	free(tmpstr);


	panel = CreateWindowEx(WS_EX_CONTROLPARENT, PanelClass.lpszClassName, "", WS_VISIBLE | WS_CHILD, 0, 290, WND_WIDTH, 30, mainwnd, (HMENU)6, hInst, NULL);

	tmpstr = fromUTF8(PAYPAL_LINK_TEXT);
	lbPaypal = CreateWindowEx(0, "Button", tmpstr, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 40, 4, 150, 20, panel, (HMENU)7, hInst, NULL);
	free(tmpstr);

	tmpstr = fromUTF8(GP_LINK_TEXT);
	lbGP = CreateWindowEx(0, "Button", tmpstr, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 280, 4, 150, 20, panel, (HMENU)8, hInst, NULL);
	free(tmpstr);


	//+++ create font +++
	hFont1 = CreateFont(-12, 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, "Tahoma");

	if (hFont1 == NULL) {
		hFont1 = CreateFont(-12, 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");
	}

	if (hFont1) {
		SendMessage(mainwnd, WM_SETFONT, (DWORD)hFont1, 0);
		SendMessage(lbInfo, WM_SETFONT, (DWORD)hFont1, 0);
		SendMessage(lbStatus, WM_SETFONT, (DWORD)hFont1, 0);
		SendMessage(btnStart, WM_SETFONT, (DWORD)hFont1, 0);
		SendMessage(lbCredits, WM_SETFONT, (DWORD)hFont1, 0);
		SendMessage(lbPaypal, WM_SETFONT, (DWORD)hFont1, 0);
		SendMessage(lbGP, WM_SETFONT, (DWORD)hFont1, 0);
	}

	UpdateWindow(mainwnd);
	this->closeBlocked = 0;
	this->worker = new AbsintheWorker(this);
}

void AbsintheMainWnd::run(void)
{
	MSG lpMsg;

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		if (!IsDialogMessage(this->mainwnd, &lpMsg)) {
			TranslateMessage(&lpMsg);
			DispatchMessage(&lpMsg);
		} else {
			if (lpMsg.hwnd != this->mainwnd) {
				WindowProc(lpMsg.hwnd, lpMsg.message, lpMsg.wParam, lpMsg.lParam);
			}
		}
	}
}
