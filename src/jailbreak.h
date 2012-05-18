#ifndef __JAILBREAK_H
#define __JAILBREAK_H

#define CONNECTION_NAME "jailbreak"
#define VPN_TRIGGER_ADDRESS "127.0.0.1"
#define AFCTMP "HackStore"

struct dev_vmaddr {
	const char* product;
	const char* build;
	uint32_t vmaddr;
};

static struct dev_vmaddr devices_vmaddr_libcopyfile[] = {
	// iOS 5.0
	{ "iPad1,1", "9A334", 0x327b2000 },
	{ "iPhone2,1", "9A334", 0x33748000 },
	{ "iPhone3,1", "9A334", 0x355f4000 },
	{ "iPhone4,1", "9A334", 0x36f60000 },
	// iOS 5.0.1
	{ "iPad1,1", "9A405", 0x3012f000 },
	{ "iPad2,1", "9A405", 0x37d2a000 },
	{ "iPad2,2", "9A405", 0x37bfc000 },
	{ "iPad2,3", "9A405", 0x36c23000 },
	{ "iPhone2,1", "9A405", 0x34a52000 },
	{ "iPhone3,1", "9A405", 0x30654000 },
	{ "iPhone3,3", "9A405", 0x323b1000 },
	{ "iPhone4,1", "9A405", 0x31f54000 },
	{ "iPhone4,1", "9A406", 0x31cfb000 },
	{ "iPod3,1", "9A405", 0x35202000 },
	{ "iPod4,1", "9A405", 0x30c29000 },
	{ 0, 0, 0 }
};

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*status_cb_t)(const char* message, int progress);

int jb_device_is_supported(const char* product, const char* build);
int jb_check_consistency(const char* product, const char* build);
void jb_device_event_cb(const idevice_event_t *event, void *user_data);
void jb_signal_handler(int sig);
int jailbreak(const char* uuid, status_cb_t status_cb);

#ifdef __cplusplus
};
#endif

#endif
