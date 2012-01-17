#include "AbsintheJailbreaker.h"
#include "jailbreak.h"

static AbsintheJailbreaker* self;

static void status_cb(const char* message, int progress)
{
	self->statusCallback(message, progress);
}

AbsintheJailbreaker::AbsintheJailbreaker(AbsintheWorker* worker)
	: wxThread(wxTHREAD_JOINABLE), worker(worker)
{
	self = this;
}

void AbsintheJailbreaker::statusCallback(const char* message, int progress)
{
	worker->processStatus(message, progress);
}

wxThread::ExitCode AbsintheJailbreaker::Entry(void)
{
	jailbreak(worker->getUUID(), status_cb);

	const char* error = "Done!";

	worker->processFinished(error);
	return 0;
}
