#ifndef __ABSINTHE_MAIN_WND_H
#define __ABSINTHE_MAIN_WND_H

#include "version.h"

/* gui definitions */
#define APPNAME "Chronic-Dev Absinthe"
#define WND_TITLE APPNAME " - Version " ABSINTHE_VERSION_STRING
#define WND_WIDTH 480
#define WND_HEIGHT 350

#define INFO_LABEL_TEXT "Welcome to Absinthe iOS 5.1.1 untethered jailbreak!\n\nPlease make a backup of your device before using this tool. We don't expect any issues, but we aren't responsible if anything happens."
#define BTN_START_TEXT "Jailbreak"
#define CREDITS_LABEL_TEXT "Chronic-Dev Absinthe © 2011-2012 Chronic-Dev Team.\n5.1.x exploits by: @pod2g, @planetbeing, and @pimskeks\n5.0.x exploits by: @pod2g, @planetbeing, @saurik, @pimskeks,\n@p0sixninja, @MuscleNerd, and @xvolks.\nArtwork by @iOPK. GUI by Hanéne Samara & @pimskeks."
#define PAYPAL_LINK_TEXT "Support us (PayPal)"
#define PAYPAL_LINK_URL "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4U6DQGJ2NRVUN"
#define GP_LINK_TEXT "http://greenpois0n.com/"
#define GP_LINK_URL "http://greenpois0n.com/"

enum {
	mb_OK = 1 << 0,
	mb_CANCEL = 1 << 1,
	mb_YES_NO = 1 << 2,
	mb_OK_CANCEL = 1 << 3,
	mb_ICON_INFO = 1 << 8,
	mb_ICON_WARNING = 1 << 9,
	mb_ICON_QUESTION = 1 << 10,
	mb_ICON_ERROR = 1 << 11
};

#define mb_YES mb_OK
#define mb_NO mb_CANCEL

#ifdef __linux__
#include <gtk/gtk.h>
#define WIDGET_TYPE GtkWidget*
#endif

#ifdef __APPLE__
#define WIDGET_TYPE id
#endif

#ifdef WIN32
#include <windows.h>
#define WIDGET_TYPE HWND
#endif

#ifndef WIDGET_TYPE
#define WIDGET_TYPE void*
#endif

#include "AbsintheWorker.h"

class AbsintheWorker;

class AbsintheMainWnd
{
public:
	WIDGET_TYPE mainwnd;
	WIDGET_TYPE progressBar;
	WIDGET_TYPE btnStart;
	WIDGET_TYPE lbStatus;
	AbsintheWorker* worker;

	int closeBlocked;
	AbsintheMainWnd(int* pargc, char*** pargv);
	void run(void);

	int msgBox(const char* message, const char* caption, int style);

	void setButtonEnabled(int enabled);
	void setStatusText(const char* text);
	void setProgress(int percentage);

	void handleStartClicked(void* data);
	bool onClose(void* data);
};
#endif
