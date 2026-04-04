#ifndef __JAILBREAK_H
#define __JAILBREAK_H

#define AFCTMP "HackStore"

#include "device_compat.h"
#include "vpn_config.h"

/* Legacy aliases kept for backward compatibility */
#define CONNECTION_NAME    VPN_CONNECTION_NAME

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*status_cb_t)(const char* message, int progress);

void jb_device_event_cb(const idevice_event_t *event, void *user_data);
void jb_signal_handler(int sig);
int jailbreak(const char* uuid, status_cb_t status_cb);

#ifdef __cplusplus
};
#endif

#endif
