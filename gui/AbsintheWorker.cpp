#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include "AbsintheWorker.h"
#include "AbsintheJailbreaker.h"
#include "iTunesKiller.h"
#include "device_types.h"
#include "jailbreak.h"

extern "C" {
typedef int16_t userpref_error_t;
extern userpref_error_t userpref_remove_device_public_key(const char *udid);
}

static int detection_blocked = 0;
static AbsintheWorker* self;

static const char* getDeviceName(const char* productType)
{
	int i = 0;
	while (device_types[i].productType) {
		if (strcmp(device_types[i].productType, productType) == 0) {
			return device_types[i].displayName;
		}
		i++;
	}
	return productType;
}

static void device_event_cb(const idevice_event_t* event, void* userdata)
{
	if (!detection_blocked) {
		self->DeviceEventCB(event, userdata);
	}
	jb_device_event_cb(event, (void*)event->udid);
}

AbsintheWorker::AbsintheWorker(AbsintheMainWnd* main)
	: mainWnd(main), device_count(0)
{
	self = this;

	this->current_udid = NULL;

	struct stat st;
	int quick_check = 0;
	memset(&st, '\0', sizeof(struct stat));
	if ((stat("data/9B208", &st) == 0) && S_ISDIR(st.st_mode)) {
		quick_check++;
	}
	memset(&st, '\0', sizeof(struct stat));
	if ((stat("data/9B206", &st) == 0) && S_ISDIR(st.st_mode)) {
		quick_check++;
	}
	memset(&st, '\0', sizeof(struct stat));
	if ((stat("data/9A406", &st) == 0) && S_ISDIR(st.st_mode)) {
		quick_check++;
	}
	memset(&st, '\0', sizeof(struct stat));
	if ((stat("data/9A405", &st) == 0) && S_ISDIR(st.st_mode)) {
		quick_check++;
	}
	memset(&st, '\0', sizeof(struct stat));
	if ((stat("data/9A334", &st) == 0) && S_ISDIR(st.st_mode)) {
		quick_check++;
	}
	memset(&st, '\0', sizeof(struct stat));
	if ((stat("data/common", &st) == 0) && S_ISDIR(st.st_mode)) {
		quick_check++;
	}
	if (quick_check != 6) {
		this->mainWnd->setStatusText("ERROR: Could not find required files. Make sure you extracted the entire package!");
		return;
	}

	this->checkDevice();

	idevice_event_subscribe(&device_event_cb, NULL);
}

AbsintheWorker::~AbsintheWorker(void)
{
	idevice_event_unsubscribe();
	if (this->current_udid) {
		free(this->current_udid);
	}
}

void AbsintheWorker::setUDID(const char* udid)
{
	if (this->current_udid) {
		free(this->current_udid);
		this->current_udid = NULL;
	}
	if (udid) {
		this->current_udid = strdup(udid);
	}
}

char* AbsintheWorker::getUDID(void)
{
	return current_udid;
}

void AbsintheWorker::DeviceEventCB(const idevice_event_t *event, void *user_data)
{
	if (event->event == IDEVICE_DEVICE_ADD) {
		this->device_count++;
		this->checkDevice();
	} else if (event->event == IDEVICE_DEVICE_REMOVE) {
		this->device_count--;
		this->checkDevice();
	}
}

void AbsintheWorker::checkDevice()
{
	AbsintheMainWnd* mainwnd = (AbsintheMainWnd*)this->mainWnd;
	char str[256];

	this->setUDID(NULL);

	if (this->device_count == 0) {
		mainwnd->setButtonEnabled(0);
		mainwnd->setProgress(0);
		mainwnd->setStatusText("Plug in your iDevice to begin.");
	} else if (this->device_count == 1) {
		idevice_t dev = NULL;
		idevice_error_t ierr = idevice_new(&dev, NULL);
		if (ierr != IDEVICE_E_SUCCESS) {
			sprintf(str, "Error detecting device type (idevice error %d)", ierr);
			mainwnd->setStatusText(str);
			return;
		}

		lockdownd_client_t client = NULL;
		lockdownd_error_t lerr = lockdownd_client_new_with_handshake(dev, &client, "absinthe");
		if (lerr == LOCKDOWN_E_PASSWORD_PROTECTED) {
			lockdownd_client_free(client);
			idevice_free(dev);
			sprintf(str, "ERROR: Device has a passcode set! If a passcode is set, the jailbreak procedure will most likely fail. Unplug device, go to Settings and DISABLE THE PASSCODE, then plug it back in.");
			mainwnd->setStatusText(str);
			return;
		} else if (lerr == LOCKDOWN_E_INVALID_HOST_ID) {
			lerr = lockdownd_unpair(client, NULL);
			if (lerr == LOCKDOWN_E_SUCCESS) {
				char *devudid = NULL;
				idevice_get_udid(dev, &devudid);
				if (devudid) {
					userpref_remove_device_public_key(devudid);
					free(devudid);
				}
			}
			lockdownd_client_free(client);
			idevice_free(dev);
			sprintf(str, "Device detection failed. Please unplug device and plug it back in. Then it should work.");
			mainwnd->setStatusText(str);
			return;
		} else if (lerr == LOCKDOWN_E_SSL_ERROR) {
			lockdownd_client_t newl = NULL;
			lockdownd_client_new(dev, &newl, "absinthe");
			if (newl) {
				plist_t device_public_key = NULL;
				lockdownd_get_value(newl, NULL, "DevicePublicKey", &device_public_key);
				if (device_public_key && (plist_get_node_type(device_public_key) == PLIST_DATA)) {
					char* testdata = NULL;
					uint64_t testsize = 0;
					plist_get_data_val(device_public_key, &testdata, &testsize);
					const char chk[] = "-----BEGIN RSA PUBLIC KEY-----";
					if (memcmp(testdata, chk, strlen(chk)) == 0) {
						lerr = lockdownd_unpair(newl, NULL);
						if (lerr == LOCKDOWN_E_SUCCESS) {
							char *devudid = NULL;
							idevice_get_udid(dev, &devudid);
							if (devudid) {
								userpref_remove_device_public_key(devudid);
								free(devudid);
							}
						}
						lockdownd_client_free(newl);
						idevice_free(dev);
						sprintf(str, "Device detection failed. Please unplug device and plug it back in.");
						mainwnd->setStatusText(str);
						return;
					}
				}
				lockdownd_client_free(newl);
			}
			idevice_free(dev);
			sprintf(str, "Error detecting device (lockdown error %d)", lerr);
			mainwnd->setStatusText(str);
			return;
		} else if (lerr != LOCKDOWN_E_SUCCESS) {
			idevice_free(dev);
			sprintf(str, "Error detecting device (lockdown error %d)", lerr);
			mainwnd->setStatusText(str);
			return;
		}

		plist_t node = NULL;
		char* productType = NULL;
		char* productVersion = NULL;
		char* buildVersion = NULL;

		node = NULL;
		lerr = lockdownd_get_value(client, NULL, "ProductType", &node);
		if (node) {
			plist_get_string_val(node, &productType);
			plist_free(node);
		}
		if ((lerr != LOCKDOWN_E_SUCCESS) || !productType) {
			lockdownd_client_free(client);
			idevice_free(dev);
			sprintf(str, "Error getting product type (lockdown error %d)", lerr);
			mainwnd->setStatusText(str);
			return;
		}

		node = NULL;
		lerr = lockdownd_get_value(client, NULL, "ProductVersion", &node);
		if (node) {
			plist_get_string_val(node, &productVersion);
			plist_free(node);
		}
		if ((lerr != LOCKDOWN_E_SUCCESS) || !productVersion) {
			free(productType);
			lockdownd_client_free(client);
			idevice_free(dev);
			sprintf(str, "Error getting product version (lockdownd error %d)", lerr);
			mainwnd->setStatusText(str);
			return;
		}

		node = NULL;
		lerr = lockdownd_get_value(client, NULL, "BuildVersion", &node);
		if (node) {
			plist_get_string_val(node, &buildVersion);
			plist_free(node);
		}
		if ((lerr != LOCKDOWN_E_SUCCESS) || !buildVersion) {
			free(productType);
			free(productVersion);
			lockdownd_client_free(client);
			idevice_free(dev);
			sprintf(str, "Error getting build version (lockdownd error %d)", lerr);
			mainwnd->setStatusText(str);
			return;
		}

		if (!jb_device_is_supported(productType, buildVersion)) {
			mainwnd->setStatusText("Sorry, the attached device is not supported.");
			free(productType);
			free(productVersion);
			free(buildVersion);
			lockdownd_client_free(client);
			idevice_free(dev);
			return;
		}

		int cc = jb_check_consistency(productType, buildVersion);
		if (cc == 0) {
			// Consistency check passed
		} else if (cc == -1) {
			mainwnd->setStatusText("ERROR: Consistency check failed: attached device not supported");
			free(productType);
			free(productVersion);
			free(buildVersion);
			lockdownd_client_free(client);
			idevice_free(dev);
			return;
		} else if (cc == -2) {
			mainwnd->setStatusText("ERROR: Consistency check failed: could not find required files");
			free(productType);
			free(productVersion);
			free(buildVersion);
			lockdownd_client_free(client);
			idevice_free(dev);
			return;
		} else {
			mainwnd->setStatusText("ERROR: Consistency check failed: unknown error");
			free(productType);
			free(productVersion);
			free(buildVersion);
			lockdownd_client_free(client);
			idevice_free(dev);
			return;
		}

		node = NULL;
		lockdownd_get_value(client, NULL, "PasswordProtected", &node);
		if (node) {
			uint8_t pcenabled = 0;
			plist_get_bool_val(node, &pcenabled);
			plist_free(node);
			if (pcenabled) {
				sprintf(str, "ERROR: Device has a passcode set! If a passcode is set, the jailbreak procedure will most likely fail. Unplug device, go to Settings and DISABLE THE PASSCODE, then plug it back in.");
				mainwnd->setStatusText(str);
				return;
			}
		}

		sprintf(str, "%s with iOS %s (%s) detected. Click the button to begin.", getDeviceName(productType), productVersion, buildVersion);
		mainwnd->setStatusText(str);

		int ready_to_go = 1;

		plist_t pl = NULL;
		lockdownd_get_value(client, NULL, "ActivationState", &pl);
		if (pl && plist_get_node_type(pl) == PLIST_STRING) {
			char* as = NULL;
			plist_get_string_val(pl, &as);
			plist_free(pl);
			if (as) {
				if (strcmp(as, "Unactivated") == 0) {
					ready_to_go = 0;
					mainwnd->msgBox("The attached device is not activated. You need to activate it before it can be used with Absinthe.", "Error", mb_OK | mb_ICON_ERROR);
				}
				free(as);
			}
		}

		pl = NULL;
		lockdownd_get_value(client, "com.apple.mobile.backup", "WillEncrypt", &pl);
		lockdownd_client_free(client);

		if (pl && plist_get_node_type(pl) == PLIST_BOOLEAN) {
			uint8_t c = 0;
			plist_get_bool_val(pl, &c);
			plist_free(pl);
			if (c) {
				ready_to_go = 0;
				mainwnd->msgBox("The attached device has a backup password set. You need to disable the backup password in iTunes before you can continue.\nStart iTunes, remove the backup password and start this Program again.", "Error", mb_OK | mb_ICON_ERROR);
			}
		}

		if (ready_to_go) {
			char* udid = NULL;
			idevice_get_udid(dev, &udid);
			if (udid) {
				this->setUDID(udid);
				free(udid);
			}
			mainwnd->setButtonEnabled(1);
		}
		idevice_free(dev);

		free(productType);
		free(productVersion);
		free(buildVersion);
	} else {
		mainwnd->setButtonEnabled(0);
		mainwnd->setStatusText("Please attach only one device.");
	}
}

void AbsintheWorker::processStart(void)
{
	AbsintheMainWnd* mainwnd = (AbsintheMainWnd*)this->mainWnd;

	detection_blocked = 1;
	mainwnd->closeBlocked = 1;

#if defined(__APPLE__) || defined(WIN32)
	iTunesKiller* ik = new iTunesKiller(&detection_blocked);
	ik->Start();
#endif

	AbsintheJailbreaker* jb = new AbsintheJailbreaker(this);
	jb->Start();
}

void AbsintheWorker::processStatus(const char* msg, int progress)
{
	AbsintheMainWnd* mainwnd = (AbsintheMainWnd*)this->mainWnd;
	if (msg) {
		mainwnd->setStatusText(msg);
	}
	mainwnd->setProgress(progress);
}

void AbsintheWorker::processFinished(const char* error)
{
	AbsintheMainWnd* mainwnd = (AbsintheMainWnd*)this->mainWnd;

	detection_blocked = 0;
	mainwnd->closeBlocked = 0;
}
