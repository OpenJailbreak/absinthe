#ifndef __JAILBREAK_H
#define __JAILBREAK_H

typedef void (*status_cb_t)(const char* message, int progress);

void jb_device_event_cb(const idevice_event_t *event, void *user_data);
void jb_signal_handler(int sig);
int jailbreak(const char* uuid, status_cb_t status_cb);

#endif
