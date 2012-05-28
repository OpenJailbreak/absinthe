#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "AbsintheJailbreaker.h"
#include "jailbreak.h"

static AbsintheJailbreaker* self;

static void status_cb(const char* message, int progress)
{
	self->statusCallback(message, progress);
}

AbsintheJailbreaker::AbsintheJailbreaker(AbsintheWorker* worker)
	: worker(worker)
{
	self = this;
}

void AbsintheJailbreaker::statusCallback(const char* message, int progress)
{
	worker->processStatus(message, progress);
}

void* AbsintheJailbreaker::Entry(void* data)
{
	char* udid = strdup(worker->getUDID());
	jailbreak(udid, status_cb);
	free(udid);

	const char* error = "Done!";

	worker->processFinished(error);
	return 0;
}

static void* thread_func(void* data)
{
	return self->Entry(data);
}

void AbsintheJailbreaker::Start(void)
{
#ifdef WIN32
	this->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_func, NULL, 0, NULL);
#else
	pthread_create(&this->thread, NULL, thread_func, NULL);
#endif
}
