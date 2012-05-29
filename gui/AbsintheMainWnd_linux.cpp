#include <gtk/gtk.h>
#include "AbsintheMainWnd.h"

int AbsintheMainWnd::msgBox(const char* message, const char* caption, int style)
{
	int res = 0;

	// get message type
	GtkMessageType mtype = GTK_MESSAGE_OTHER;
	if (style & mb_ICON_INFO) {
		mtype = GTK_MESSAGE_INFO;
	} else if (style & mb_ICON_WARNING) {
		mtype = GTK_MESSAGE_WARNING;
	} else if (style & mb_ICON_QUESTION) {
		mtype = GTK_MESSAGE_QUESTION;
	} else if (style & mb_ICON_ERROR) {
		mtype = GTK_MESSAGE_ERROR;
	}

	// get button type(s)
	GtkButtonsType btype = GTK_BUTTONS_OK;
	if (style & mb_OK) {
		btype = GTK_BUTTONS_OK;
	} else if (style & mb_CANCEL) {
		btype = GTK_BUTTONS_CANCEL;
	} else if (style & mb_OK_CANCEL) {
		btype = GTK_BUTTONS_OK_CANCEL;
	} else if (style & mb_YES_NO) {
		btype = GTK_BUTTONS_YES_NO;
	}

	GtkWidget* msgbox = gtk_message_dialog_new(GTK_WINDOW(this->mainwnd), GTK_DIALOG_MODAL, mtype, btype, "%s", caption);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(msgbox), "%s", message);
	gdk_threads_enter();
	int answer = gtk_dialog_run(GTK_DIALOG(msgbox));
	gtk_widget_destroy(msgbox);
	gdk_threads_leave();
	switch (answer) {
	case GTK_RESPONSE_OK:
		return mb_OK;
	case GTK_RESPONSE_CANCEL:
		return mb_CANCEL;
	case GTK_RESPONSE_YES:
		return mb_YES;
	case GTK_RESPONSE_NO:
		return mb_NO;
	default:
		return -1;
	}
}

void AbsintheMainWnd::setButtonEnabled(int enabled)
{
	gtk_widget_set_sensitive(this->btnStart, enabled);
}

void AbsintheMainWnd::setStatusText(const char* text)
{
	gtk_label_set_text(GTK_LABEL(this->lbStatus), (gchar*)text);
	gtk_widget_queue_draw (this->lbStatus);
}

void AbsintheMainWnd::setProgress(int percentage)
{	
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(this->progressBar), (double)percentage / 100.0f);
	gtk_widget_queue_draw (this->progressBar);
}

void AbsintheMainWnd::handleStartClicked(void* data)
{
	this->setButtonEnabled(0);
	this->setProgress(0);
	this->worker->processStart();
}

bool AbsintheMainWnd::onClose(void* data)
{
	if (this->closeBlocked) {
		return TRUE;
	}
	return FALSE;
}

static gchar* g_strreplace (const gchar *string, const gchar *search, const gchar *replacement)
{
	gchar *str, **arr;

	g_return_val_if_fail (string != NULL, NULL);
	g_return_val_if_fail (search != NULL, NULL);

	if (replacement == NULL)
		replacement = "";

	arr = g_strsplit (string, search, -1);
	if (arr != NULL && arr[0] != NULL)
		str = g_strjoinv (replacement, arr);
	else
		str = g_strdup (string);

	g_strfreev (arr);

	return str;
}

extern "C" {

static void start_clicked(GtkWidget* widget, gpointer data)
{
	AbsintheMainWnd* _this = (AbsintheMainWnd*)data;
	_this->handleStartClicked(widget);
}

static gboolean delete_event(GtkWidget* widget, gpointer data)
{
	AbsintheMainWnd* _this = (AbsintheMainWnd*)data;
	return _this->onClose(widget);
}

static void destroy_event(GtkWidget* widget, gpointer data)
{
	gtk_main_quit();
}

}

AbsintheMainWnd::AbsintheMainWnd(int* pargc, char*** pargv)
{
	g_thread_init(NULL);

	gdk_threads_init();

	gtk_init(pargc, pargv);

	mainwnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(mainwnd), 0);
	gtk_widget_set_size_request(mainwnd, WND_WIDTH, WND_HEIGHT);
	gtk_window_set_position(GTK_WINDOW(mainwnd), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(mainwnd), WND_TITLE);
	gtk_window_set_icon_from_file(GTK_WINDOW(mainwnd), "data/icon.png", NULL);
	gtk_container_set_border_width(GTK_CONTAINER(mainwnd), 10);

	g_signal_connect(mainwnd, "delete-event", G_CALLBACK(delete_event), this);
	g_signal_connect(mainwnd, "destroy", G_CALLBACK(destroy_event), NULL);

	GtkWidget* vbox = gtk_vbox_new(0, 0);

	PangoFontDescription* small_font = pango_font_description_new();
	pango_font_description_set_size(small_font, 11000);

	PangoFontDescription* smaller_font = pango_font_description_new();
	pango_font_description_set_size(smaller_font, 10500);

	GtkWidget* lbTop = gtk_label_new(INFO_LABEL_TEXT);
	gtk_label_set_line_wrap(GTK_LABEL(lbTop), 1);
	gtk_widget_set_size_request(lbTop, WND_WIDTH-20, 76);
	gtk_misc_set_alignment(GTK_MISC(lbTop), 0.0, 0.0);
	gtk_widget_modify_font(lbTop, small_font);

	lbStatus = gtk_label_new(NULL);
	gtk_label_set_line_wrap(GTK_LABEL(lbStatus), 1);
	gtk_widget_set_size_request(lbStatus, WND_WIDTH-20, 82);
	gtk_misc_set_alignment(GTK_MISC(lbStatus), 0.0, 0.0);
	gtk_widget_modify_font(lbStatus, small_font);

	progressBar = gtk_progress_bar_new();
	GtkWidget* pbalign = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_widget_set_size_request(progressBar, 300, 17);
	gtk_container_add(GTK_CONTAINER(pbalign), progressBar);

	btnStart = gtk_button_new_with_label(BTN_START_TEXT);
	gtk_widget_modify_font(btnStart, smaller_font);
	gtk_widget_set_sensitive(btnStart, 0);
	g_signal_connect(btnStart, "clicked", G_CALLBACK(start_clicked), this);

	GtkWidget* hbox = gtk_hbox_new(0, 20);
	gtk_box_pack_start(GTK_BOX(hbox), pbalign, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), btnStart, 0, 0, 0);

	GtkWidget* pbbtnalign = gtk_alignment_new(0.5, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(pbbtnalign), hbox);

	GtkWidget* lbCredits = gtk_label_new(CREDITS_LABEL_TEXT);
	gtk_label_set_line_wrap(GTK_LABEL(lbCredits), 1);
	gtk_label_set_justify(GTK_LABEL(lbCredits), GTK_JUSTIFY_CENTER);
	gtk_widget_modify_font(lbCredits, smaller_font);

	GtkWidget* lbPaypal = gtk_label_new(NULL);
	gchar* url = g_strreplace("<a href=\"" PAYPAL_LINK_URL "\">" PAYPAL_LINK_TEXT "</a>", "&", "&amp;");
	gtk_label_set_markup(GTK_LABEL(lbPaypal), url);
	g_free(url);
	gtk_misc_set_alignment(GTK_MISC(lbPaypal), 0.5, 0.0);
	gtk_label_set_track_visited_links(GTK_LABEL(lbPaypal), FALSE);
	gtk_widget_modify_font(lbPaypal, small_font);

	GtkWidget* lbGP = gtk_label_new(NULL);
	url = g_strreplace("<a href=\"" GP_LINK_URL "\">" GP_LINK_TEXT "</a>", "&", "&amp;");
	gtk_label_set_markup(GTK_LABEL(lbGP), url);
	g_free(url);
	gtk_misc_set_alignment(GTK_MISC(lbGP), 0.5, 0.0);
	gtk_label_set_track_visited_links(GTK_LABEL(lbGP), FALSE);
	gtk_widget_modify_font(lbGP, small_font);

	GtkWidget* hbox2 = gtk_hbox_new(0, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), lbPaypal, 1, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox2), lbGP, 1, 0, 0);

	gtk_box_pack_start(GTK_BOX(vbox), lbTop, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), lbStatus, 0, 0, 10);
	gtk_box_pack_start(GTK_BOX(vbox), pbbtnalign, 0, 0, 4);
	gtk_box_pack_start(GTK_BOX(vbox), lbCredits, 0, 0, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, 0, 0, 10);

	gtk_container_add(GTK_CONTAINER(mainwnd), vbox);
	gtk_widget_show_all(mainwnd);

	this->closeBlocked = 0;
	this->worker = new AbsintheWorker(this);
}

void AbsintheMainWnd::run(void)
{
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
}
