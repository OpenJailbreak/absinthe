#include "AbsintheMainWnd.h"
#include "version.h"

AbsintheMainWnd::AbsintheMainWnd(void)
	: wxFrame(NULL, wxID_ANY, wxT(WND_TITLE " - Version " ABSINTHE_VERSION_STRING), wxDefaultPosition, wxSize(WND_WIDTH, WND_HEIGHT), (wxSTAY_ON_TOP | wxDEFAULT_FRAME_STYLE) & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
#if defined(__WXMSW__)
	SetIcon(wxICON(AppIcon));
#endif
	wxPanel* panel = new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(WND_WIDTH, WND_HEIGHT));

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
#if defined(__WXGTK__)
#define FNTSIZE 10
#endif
#if defined(__WXOSX_COCOA__)
#define FNTSIZE 12
#endif
#if defined(__WXMSW__)
#define FNTSIZE 9
#endif
	wxFont fnt(FNTSIZE, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	wxStaticText* lbTop = new wxStaticText(panel, wxID_ANY, wxT("Welcome to Absinthe iOS 5.0/5.0.1 untethered A5 jailbreak!\n\nPlease make a backup of your device before using this tool. We don't expect any issues, but we aren't responsible if anything happens."), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE | wxALIGN_LEFT);
	lbTop->SetFont(fnt);
	lbTop->Wrap(WND_WIDTH-20);

	lbStatus = new wxStaticText(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
	lbStatus->SetFont(fnt);

	progressBar = new wxGauge(panel, wxID_ANY, 100, wxDefaultPosition, wxSize(300,17), wxGA_HORIZONTAL | wxGA_SMOOTH);
	btnStart = new wxButton(panel, 1111, wxT("Jailbreak"));
	btnStart->Enable(0);
	Connect(1111, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AbsintheMainWnd::handleStartClicked));

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(progressBar, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);
	hbox->Add(btnStart, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);

	wxStaticText* lbCredits = new wxStaticText(panel, wxID_ANY, wxT("Chronic-Dev Absinthe © 2011-2012 Chronic-Dev Team.\nExploits by: @pod2g, @planetbeing, @saurik, @pimskeks,\n@p0sixninja, @MuscleNerd, and @xvolks.\nArtwork by @iOPK. GUI by Hanéne Samara && @pimskeks."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxST_NO_AUTORESIZE);

	lbCredits->SetFont(fnt);

	wxStaticText* lbPaypal = new wxStaticText(panel, wxID_ANY, wxT("Support Us (PayPal)"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	lbPaypal->SetForegroundColour(wxColour(wxT("BLUE")));
	lbPaypal->Connect(wxEVT_LEFT_DOWN, wxCommandEventHandler(AbsintheMainWnd::PaypalClicked));
	lbPaypal->SetFont(fnt);

	wxStaticText* lbGP = new wxStaticText(panel, wxID_ANY, wxT("http://greenpois0n.com/"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	lbGP->SetForegroundColour(wxColour(wxT("BLUE")));
	lbGP->Connect(wxEVT_LEFT_DOWN, wxCommandEventHandler(AbsintheMainWnd::GPClicked));
	lbGP->SetFont(fnt);

	wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);
	hbox2->Add(lbPaypal, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);
	hbox2->AddSpacer(50);
	hbox2->Add(lbGP, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);

	vbox->Add(lbTop, 0, wxEXPAND | wxALL, 10);
	vbox->Add(lbStatus, 1, wxEXPAND | wxALL, 10);
	vbox->Add(hbox, 0, wxCENTER | wxALL, 4);
	vbox->Add(lbCredits, 0, wxCENTER | wxALL, 0);
	vbox->Add(hbox2, 0, wxCENTER | wxALL, 4);

	panel->SetSizer(vbox);

	Centre();

	this->worker = new AbsintheWorker((AbsintheMainWnd*)this);

	this->closeBlocked = 0;
	this->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(AbsintheMainWnd::OnClose));
}

#define THREAD_SAFE(X) \
	if (!wxIsMainThread()) { \
		wxMutexGuiEnter(); \
		X; \
		wxMutexGuiLeave(); \
	} else { \
		X; \
	}

void AbsintheMainWnd::PaypalClicked(wxCommandEvent& event)
{
	wxLaunchDefaultBrowser(wxT("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=DPFUPCEAYUD4L"), 1);
}

void AbsintheMainWnd::GPClicked(wxCommandEvent& event)
{
	wxLaunchDefaultBrowser(wxT("http://greenpois0n.com/"), 1);
}

int AbsintheMainWnd::msgBox(const wxString& message, const wxString& caption, int style)
{
	int res;
	THREAD_SAFE(res = wxMessageBox(message, caption, style));
	return res;
}

void AbsintheMainWnd::setButtonEnabled(int enabled)
{
	THREAD_SAFE(this->btnStart->Enable(enabled));
}

void AbsintheMainWnd::setStatusText(const wxString& text)
{
	THREAD_SAFE(this->lbStatus->SetLabel(text));
}

void AbsintheMainWnd::setProgress(int percentage)
{
	THREAD_SAFE(this->progressBar->SetValue(percentage));
}

void AbsintheMainWnd::handleStartClicked(wxCommandEvent& WXUNUSED(event))
{
	this->setButtonEnabled(0);
	this->setProgress(0);
	this->worker->processStart();
}

void AbsintheMainWnd::OnClose(wxCloseEvent& event)
{
	if (this->closeBlocked) {
		event.Veto();
	} else {
		event.Skip();
	}
}

void AbsintheMainWnd::OnQuit(wxCommandEvent& event)
{
	Close(true);
}

