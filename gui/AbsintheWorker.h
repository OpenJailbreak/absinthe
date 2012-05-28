#ifndef __ABSINTHEWORKER_H
#define __ABSINTHEWORKER_H

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#include "AbsintheMainWnd.h"

class AbsintheMainWnd;

class AbsintheWorker
{
private:
	AbsintheMainWnd* mainWnd;
	int device_count;
	char* current_udid;
public:
	AbsintheWorker(AbsintheMainWnd* main);
	~AbsintheWorker(void);
	void setUDID(const char* udid);
	char* getUDID(void);
	void DeviceEventCB(const idevice_event_t *event, void *user_data);
	void checkDevice();
	void processStart(void);
	void processStatus(const char* msg, int progress);
	void processFinished(const char* error);
};

#endif /* __ABSINTHEWORKER_H */
