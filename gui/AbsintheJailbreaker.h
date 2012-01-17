#ifndef __ABSINTHEJAILBREAKER_H 
#define __ABSINTHEJAILBREAKER_H

#include <wx/wx.h>

#include "AbsintheWorker.h"

class AbsintheJailbreaker : public wxThread
{
private:
	AbsintheWorker* worker;
public:
	AbsintheJailbreaker(AbsintheWorker* worker);
	void statusCallback(const char* message, int progress);
	wxThread::ExitCode Entry(void);
};

#endif /* __ABSINTHEJAILBREAKER_H */
