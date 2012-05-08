#ifndef __ABSINTHEJAILBREAKER_H 
#define __ABSINTHEJAILBREAKER_H

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "AbsintheWorker.h"

class AbsintheJailbreaker
{
private:
	AbsintheWorker* worker;
#ifdef WIN32
	HANDLE thread;
#else
	pthread_t thread;
#endif

public:
	AbsintheJailbreaker(AbsintheWorker* worker);
	void Start(void);
	void statusCallback(const char* message, int progress);
	void* Entry(void* data);
};

#endif /* __ABSINTHEJAILBREAKER_H */
