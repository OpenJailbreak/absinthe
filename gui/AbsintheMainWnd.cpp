#include "AbsintheMainWnd.h"

AbsintheMainWnd::AbsintheMainWnd(void)
	: wxFrame(NULL, wxID_ANY, wxT(WND_TITLE), wxDefaultPosition, wxSize(WND_WIDTH, WND_HEIGHT), (wxSTAY_ON_TOP | wxDEFAULT_FRAME_STYLE) & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
#if defined(__WXMSW__)
	SetIcon(wxICON(AppIcon));
#endif
	wxPanel* panel = new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(WND_WIDTH, WND_HEIGHT));

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	wxStaticText* lbTop = new wxStaticText(panel, wxID_ANY, wxT("Description"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE | wxALIGN_LEFT);
	lbTop->Wrap(WND_WIDTH-20);

	lbStatus = new wxStaticText(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);

	progressBar = new wxGauge(panel, wxID_ANY, 100, wxDefaultPosition, wxSize(300,20), wxGA_HORIZONTAL | wxGA_SMOOTH);
	btnStart = new wxButton(panel, 1111, wxT("Jailbreak"));
	btnStart->Enable(0);
	Connect(1111, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AbsintheMainWnd::handleStartClicked));

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(progressBar, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);
	hbox->Add(btnStart, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);

	wxStaticText* lbCredits = new wxStaticText(panel, wxID_ANY, wxT("Chronic-Dev Absinthe © 2011-2012 Chronic-Dev Team\n"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxST_NO_AUTORESIZE);

	vbox->Add(lbTop, 0, wxEXPAND | wxALL, 10);
	vbox->Add(lbStatus, 1, wxEXPAND | wxALL, 10);
	vbox->Add(hbox, 0, wxCENTER | wxALL, 10);
	vbox->Add(lbCredits, 0, wxCENTER | wxALL, 10);

	panel->SetSizer(vbox);

	Centre();

	this->worker = new AbsintheWorker((AbsintheMainWnd*)this);
}

#define THREAD_SAFE(X) \
	if (!wxIsMainThread()) { \
		wxMutexGuiEnter(); \
		X; \
		wxMutexGuiLeave(); \
	} else { \
		X; \
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

void AbsintheMainWnd::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

