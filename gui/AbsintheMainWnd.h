#ifndef __ABSINTHEMAINWND_H
#define __ABSINTHEMAINWND_H

#include <wx/wx.h>

#include "AbsintheWorker.h"

#define WND_TITLE "Chronic-Dev Absinthe"
#define WND_WIDTH 480
#define WND_HEIGHT 350

class AbsintheWorker;

class AbsintheMainWnd : public wxFrame
{
private:
	wxStaticText* lbStatus;
	wxButton* btnStart;
	wxGauge* progressBar;
	AbsintheWorker* worker;

public:
	bool closeBlocked;
	AbsintheMainWnd(void);

	void PaypalClicked(wxCommandEvent& event);
	void GPClicked(wxCommandEvent& event);

	int msgBox(const wxString& message, const wxString& caption, int style);

	void setButtonEnabled(int enabled);
	void setStatusText(const wxString& text);
	void setProgress(int percentage);

	void handleStartClicked(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnQuit(wxCommandEvent& event);
};

enum {
	ID_QUIT = 1,
	ID_ABOUT,
};

#endif /* __ABSINTHEMAINWND_H */

