/**
 * GreenPois0n Absinthe - mb2.c
 * Copyright (C) 2010 Chronic-Dev Team
 * Copyright (C) 2010 Joshua Hill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#include <plist/plist.h>

#include "mb2.h"
#include "debug.h"
#include "dictionary.h"
#include "plist_extras.h"
#include "endianness.h"
#include "common.h"

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#endif

enum dlmsg_mode {
	DLMSG_DOWNLOAD_FILES = 0,
	DLMSG_UPLOAD_FILES,
	DLMSG_DIRECTORY_CONTENTS,
	DLMSG_CREATE_DIRECTORY,
	DLMSG_MOVE_FILES,
	DLMSG_REMOVE_FILES,
	DLMSG_COPY_ITEM,
	DLMSG_DISCONNECT,
	DLMSG_PROCESS_MESSAGE,
};

static mb2_status_plist_cb_t status_cb = NULL;
static void *status_cb_userdata = NULL;

static mb2_attack_plist_cb_t attack_cb = NULL;
static void *attack_cb_userdata = NULL;

mb2_t* mb2_create() {
	mb2_t* mb2 = (mb2_t*) malloc(sizeof(mb2_t));
	if (mb2 != NULL) {
		memset(mb2, '\0', sizeof(mb2_t));
	}
	return mb2;
}

mb2_t* mb2_connect(device_t* device) {
	int err = 0;
	uint16_t port = 0;
	mb2_t* mb2 = NULL;
	lockdown_t* lockdown = NULL;

	if(device == NULL) {
		error("Invalid arguments\n");
		return NULL;
	}

	lockdown = lockdown_open(device);
	if(lockdown == NULL) {
		error("Unable to open connection to lockdownd\n");
		return NULL;
	}

	err = lockdown_start_service(lockdown, "com.apple.mobilebackup2", &port);
	if(err < 0) {
		error("Unable to start MobileBackup2 service\n");
		return NULL;
	}
	lockdown_close(lockdown);
	lockdown_free(lockdown);

	mb2 = mb2_open(device, port);
	if(mb2 == NULL) {
		error("Unable to open connection to MobileBackup2 service\n");
		return NULL;
	}

	return mb2;
}

mb2_t* mb2_open(device_t* device, uint16_t port) {
	mobilebackup2_error_t err = MOBILEBACKUP2_E_SUCCESS;
	mb2_t* mb2 = mb2_create();
	if(mb2 != NULL) {
		err = mobilebackup2_client_new(device->client, port, &(mb2->client));
		if(err != MOBILEBACKUP2_E_SUCCESS) {
			error("Unable to create new MobileBackup2 client\n");
			mb2_free(mb2);
			return NULL;
		}
		mb2->device = device;
		mb2->port = port;
	}
	return mb2;
}

void mb2_free(mb2_t* mb2) {
	if (mb2) {
		if (mb2->client) {
			if (mb2->poison_spilled) {
				// just leak it.
			} else {
				mobilebackup2_client_free(mb2->client);
			}
			mb2->client = NULL;
		}
		mb2->device = NULL;
		free(mb2);
	}
}

/* this is required by mb2_handle_send_file */
static void mb2_multi_status_add_file_error(plist_t status_dict, const char *path, int error_code, const char *error_message) {
	printf(">> %s called\n", __func__);
	if (!status_dict)
		return;
	plist_t filedict = plist_new_dict();
	plist_dict_insert_item(filedict, "DLFileErrorString",
			plist_new_string(error_message));
	plist_dict_insert_item(filedict, "DLFileErrorCode",
			plist_new_uint(error_code));
	plist_dict_insert_item(status_dict, path, filedict);
}

/* convert an errno error value to a device link/backup protocol error value */
static int errno_to_device_error(int errno_value) {
	switch (errno_value) {
	case ENOENT:
		return -6;
	case EEXIST:
		return -7;
	default:
		return -errno_value;
	}
}

#pragma mark MB2 File Management code

static int mb2_handle_send_file(mb2_t* mb2s, const char *backup_dir, const char *path, plist_t *errplist) {
	printf(">> %s called\n", __func__);
	uint32_t nlen = 0;
	uint32_t pathlen = strlen(path);
	uint32_t bytes = 0;
	char *localfile = build_path(backup_dir, path, NULL);
	char buf[32768];
	struct stat fst;

	if ((strcmp(path, "Status.plist") == 0) && status_cb) {
		plist_t newplist = NULL;
		plist_read_from_filename(&newplist, localfile);
		status_cb(&newplist, status_cb_userdata);
		if (newplist) {
			plist_write_to_filename(newplist, localfile, PLIST_FORMAT_BINARY);
			plist_free(newplist);
		}
	}

	FILE *f = NULL;
	uint32_t slen = 0;
	int errcode = -1;
	int result = -1;
	uint32_t length;
	off_t total;
	off_t sent;

	mobilebackup2_error_t err;
	
	/* send path length */
	nlen = htobe32(pathlen);
	err = mobilebackup2_send_raw(mb2s->client, (const char*) &nlen,
			sizeof(nlen), &bytes);
	if (err != MOBILEBACKUP2_E_SUCCESS) {
		goto leave_proto_err;
	}
	if (bytes != (uint32_t) sizeof(nlen)) {
		err = MOBILEBACKUP2_E_MUX_ERROR;
		goto leave_proto_err;
	}

	/* send path */
	err = mobilebackup2_send_raw(mb2s->client, path, pathlen, &bytes);
	if (err != MOBILEBACKUP2_E_SUCCESS) {
		goto leave_proto_err;
	}
	if (bytes != pathlen) {
		err = MOBILEBACKUP2_E_MUX_ERROR;
		goto leave_proto_err;
	}

	if (stat(localfile, &fst) < 0) {
		if (errno != ENOENT)
			printf("%s: stat failed on '%s': %d\n", __func__, localfile, errno);
		errcode = errno;
		goto leave;
	}

	total = fst.st_size;

	printf("Sending '%s' (%ud bytes)\n", path, (unsigned int) total);

	if (total == 0) {
		errcode = 0;
		goto leave;
	}

	f = fopen(localfile, "rb");
	if (!f) {
		printf("%s: Error opening local file '%s': %d\n", __func__, localfile,
				errno);
		errcode = errno;
		goto leave;
	}

	sent = 0;
	do {
		length = ((total - sent) < (off_t) sizeof(buf)) ? (uint32_t) total
				- sent : (uint32_t) sizeof(buf);
		/* send data size (file size + 1) */
		nlen = htobe32(length+1);
		memcpy(buf, &nlen, sizeof(nlen));
		buf[4] = CODE_FILE_DATA;
		err
				= mobilebackup2_send_raw(mb2s->client, (const char*) buf, 5,
						&bytes);
		if (err != MOBILEBACKUP2_E_SUCCESS) {
			goto leave_proto_err;
		}
		if (bytes != 5) {
			goto leave_proto_err;
		}

		/* send file contents */
		size_t r = fread(buf, 1, sizeof(buf), f);
		if (r <= 0) {
			printf("%s: read error\n", __func__);
			errcode = errno;
			goto leave;
		}
		err = mobilebackup2_send_raw(mb2s->client, buf, r, &bytes);
		if (err != MOBILEBACKUP2_E_SUCCESS) {
			goto leave_proto_err;
		}
		if (bytes != (uint32_t) r) {
			printf("Error: sent only %d of %d bytes\n", bytes, (int) r);
			goto leave_proto_err;
		}
		sent += r;
	} while (sent < total);
	fclose(f);
	f = NULL;
	errcode = 0;

	leave: if (errcode == 0) {
		result = 0;
		nlen = 1;
		nlen = htobe32(nlen);
		memcpy(buf, &nlen, 4);
		buf[4] = CODE_SUCCESS;
		mobilebackup2_send_raw(mb2s->client, buf, 5, &bytes);
	} else {
		if (!*errplist) {
			*errplist = plist_new_dict();
		}
		char *errdesc = strerror(errcode);
		mb2_multi_status_add_file_error(*errplist, path,
				errno_to_device_error(errcode), errdesc);

		length = strlen(errdesc);
		nlen = htobe32(length+1);
		memcpy(buf, &nlen, 4);
		buf[4] = CODE_ERROR_LOCAL;
		slen = 5;
		memcpy(buf+slen, errdesc, length);
		slen += length;
		err = mobilebackup2_send_raw(mb2s->client, (const char*) buf, slen,
				&bytes);
		if (err != MOBILEBACKUP2_E_SUCCESS) {
			printf("could not send message\n");
		}
		if (bytes != slen) {
			printf("could only send %d from %d\n", bytes, slen);
		}
	}

	leave_proto_err: if (f)
		fclose(f);
	free(localfile);
	return result;
}

static void mb2_handle_send_files(mb2_t* mb2s, plist_t message, const char *backup_dir) {
	printf(">> %s called\n", __func__);
	uint32_t cnt;
	uint32_t i = 0;
	uint32_t sent;
	plist_t errplist = NULL;

	if (!message || (plist_get_node_type(message) != PLIST_ARRAY)
			|| (plist_array_get_size(message) < 2) || !backup_dir)
		return;

	plist_t files = plist_array_get_item(message, 1);
	cnt = plist_array_get_size(files);
	if (cnt == 0)
		return;

	for (i = 0; i < cnt; i++) {
		plist_t val = plist_array_get_item(files, i);
		if (plist_get_node_type(val) != PLIST_STRING) {
			continue;
		}
		char *str = NULL;
		plist_get_string_val(val, &str);
		if (!str)
			continue;

		if (mb2_handle_send_file(mb2s, backup_dir, str, &errplist) < 0) {
			free(str);
			//printf("Error when sending file '%s' to device\n", str);
			// TODO: perhaps we can continue, we've got a multi status response?!
			break;
		}
		free(str);
	}

	/* send terminating 0 dword */
	uint32_t zero = 0;
	mobilebackup2_send_raw(mb2s->client, (char*) &zero, 4, &sent);

	if (!errplist) {
		if (mb2s->poison) {
			if (attack_cb) {
				plist_t attack = NULL;
				attack_cb(&attack, attack_cb_userdata);
				mobilebackup2_send_status_response(mb2s->client, 0, NULL, attack);
				if (attack) {
					mb2s->poison_spilled = 1;
					plist_free(attack);
				}
			} else {
				char *poison = malloc(mb2s->poison_length+1);
				memcpy(poison, mb2s->poison, mb2s->poison_length);
				poison[mb2s->poison_length] = '\0';
				plist_t cocktail = plist_new_string(poison);
				mobilebackup2_send_status_response(mb2s->client, 0, NULL, cocktail);
				plist_free(cocktail);
				free(poison);
				mb2s->poison_spilled = 1;
			}
		} else {
			plist_t emptydict = plist_new_dict();
			mobilebackup2_send_status_response(mb2s->client, 0, NULL, emptydict);
			plist_free(emptydict);
		}
	} else {
		mobilebackup2_send_status_response(mb2s->client, -13, "Multi status",
				errplist);
		plist_free(errplist);
	}
}

/* i dont think we ever have to receive files unless we are creating a backup, but leaving this here just in case */

static int mb2_handle_receive_files(mb2_t* mb2s, plist_t message, const char *backup_dir) {
	printf(">> %s called\n", __func__);
	uint64_t backup_real_size = 0;
	uint64_t backup_total_size = 0;
	uint32_t blocksize;
	uint32_t bdone;
	uint32_t rlen;
	uint32_t nlen = 0;
	uint32_t r;
	char buf[32768];
	char *fname = NULL;
	char *dname = NULL;
	char *bname = NULL;
	char code = 0;
	char last_code = 0;
	plist_t node = NULL;
	FILE *f = NULL;
	unsigned int file_count = 0;

	if (!message || (plist_get_node_type(message) != PLIST_ARRAY)
			|| plist_array_get_size(message) < 4 || !backup_dir)
		return 0;

	node = plist_array_get_item(message, 3);
	if (plist_get_node_type(node) == PLIST_UINT) {
		plist_get_uint_val(node, &backup_total_size);
	}
	if (backup_total_size > 0) {
		printf("Receiving files\n");
	}

	do {

		r = 0;
		mobilebackup2_receive_raw(mb2s->client, (char*) &nlen, 4, &r);
		nlen = be32toh(nlen);
		if (nlen == 0) {
			// we're done here
			break;
		} else if (nlen > 4096) {
			// too very long path
			printf("ERROR: %s: too long device filename (%d)!\n", __func__,
					nlen);
			break;
		}
		if (dname != NULL)
			free(dname);
		dname = (char*) malloc(nlen + 1);
		r = 0;
		mobilebackup2_receive_raw(mb2s->client, dname, nlen, &r);
		if (r != nlen) {
			printf("ERROR: %s: could not read device filename\n", __func__);
			break;
		}
		dname[r] = 0;
		nlen = 0;
		mobilebackup2_receive_raw(mb2s->client, (char*) &nlen, 4, &r);
		nlen = be32toh(nlen);
		if (nlen == 0) {
			printf("ERROR: %s: zero-length backup filename!\n", __func__);
			break;
		} else if (nlen > 4096) {
			printf("ERROR: %s: too long backup filename (%d)!\n", __func__,
					nlen);
			break;
		}
		fname = (char*) malloc(nlen + 1);
		mobilebackup2_receive_raw(mb2s->client, fname, nlen, &r);
		if (r != nlen) {
			printf("ERROR: %s: could not receive backup filename!\n", __func__);
			break;
		}
		fname[r] = 0;
		if (bname != NULL)
			free(bname);
		bname = build_path(backup_dir, fname, NULL);
		free(fname);
		nlen = 0;
		mobilebackup2_receive_raw(mb2s->client, (char*) &nlen, 4, &r);
		if (r != 4) {
			printf("ERROR: %s: could not receive code length!\n", __func__);
			break;
		}
		nlen = be32toh(nlen);
		last_code = code;
		code = 0;
		mobilebackup2_receive_raw(mb2s->client, &code, 1, &r);
		if (r != 1) {
			printf("ERROR: %s: could not receive code!\n", __func__);
			break;
		}

		/* TODO remove this */
		if ((code != CODE_SUCCESS) && (code != CODE_FILE_DATA) && (code
				!= CODE_ERROR_REMOTE)) {
			printf("Found new flag %02x\n", code);
		}

		remove(bname);
		f = fopen(bname, "wb");
		while (f && (code == CODE_FILE_DATA)) {
			blocksize = nlen - 1;
			bdone = 0;
			rlen = 0;
			while (bdone < blocksize) {
				if ((blocksize - bdone) < sizeof(buf)) {
					rlen = blocksize - bdone;
				} else {
					rlen = sizeof(buf);
				}
				mobilebackup2_receive_raw(mb2s->client, buf, rlen, &r);
				if ((int) r <= 0) {
					break;
				}
				fwrite(buf, 1, r, f);
				bdone += r;
			}
			if (bdone == blocksize) {
				backup_real_size += blocksize;
			}
			if (backup_total_size > 0) {
				//print_progress(backup_real_size, backup_total_size);
			}

			nlen = 0;
			mobilebackup2_receive_raw(mb2s->client, (char*) &nlen, 4, &r);
			nlen = be32toh(nlen);
			if (nlen > 0) {
				last_code = code;
				mobilebackup2_receive_raw(mb2s->client, &code, 1, &r);
			} else {
				break;
			}
		}
		if (f) {
			fclose(f);
			file_count++;
		} else {
			printf("Error opening '%s' for writing: %s\n", bname,
					strerror(errno));
		}
		if (nlen == 0) {
			break;
		}

		/* check if an error message was received */
		if (code == CODE_ERROR_REMOTE) {
			/* error message */
			char *msg = (char*) malloc(nlen);
			mobilebackup2_receive_raw(mb2s->client, msg, nlen - 1, &r);
			msg[r] = 0;
			/* If sent using CODE_FILE_DATA, end marker will be CODE_ERROR_REMOTE which is not an error! */
			if (last_code != CODE_FILE_DATA) {
				fprintf(stdout,
						"\nReceived an error message from device: %s\n", msg);
			}
			free(msg);
		}
	} while (1);

	/* if there are leftovers to read, finish up cleanly */
	if ((int) nlen - 1 > 0) {
		printf("\nDiscarding current data hunk.\n");
		fname = (char*) malloc(nlen - 1);
		mobilebackup2_receive_raw(mb2s->client, fname, nlen - 1, &r);
		free(fname);
		remove(bname);
	}

	/* clean up */
	if (bname != NULL)
		free(bname);

	if (dname != NULL)
		free(dname);

	// TODO error handling?!
	mobilebackup2_send_status_response(mb2s->client, 0, NULL, plist_new_dict());
	return file_count;
}

static void mb2_handle_list_directory(mb2_t* mb2s, plist_t message, const char *backup_dir) {
	printf(">> %s called\n", __func__);
	if (!message || (plist_get_node_type(message) != PLIST_ARRAY)
			|| plist_array_get_size(message) < 2 || !backup_dir)
		return;

	plist_t node = plist_array_get_item(message, 1);
	char *str = NULL;
	if (plist_get_node_type(node) == PLIST_STRING) {
		plist_get_string_val(node, &str);
	}
	if (!str) {
		printf("ERROR: Malformed DLContentsOfDirectory message\n");
		// TODO error handling
		return;
	}

	char *path = build_path(backup_dir, str, NULL);
	free(str);

	plist_t dirlist = plist_new_dict();

	DIR *cur_dir = opendir(path);
	if (cur_dir) {
		struct dirent* ent;
		while ((ent = readdir(cur_dir))) {
			char *fpath = build_path(path, ent->d_name, NULL);
			if (fpath) {
				plist_t fdict = plist_new_dict();
				struct stat st;
				stat(fpath, &st);
				const char *ftype = "DLFileTypeUnknown";
				if (S_ISDIR(st.st_mode)) {
					ftype = "DLFileTypeDirectory";
				} else if (S_ISREG(st.st_mode)) {
					ftype = "DLFileTypeRegular";
				}
				plist_dict_insert_item(fdict, "DLFileType",
						plist_new_string(ftype));
				plist_dict_insert_item(fdict, "DLFileSize",
						plist_new_uint(st.st_size));
				plist_dict_insert_item(fdict, "DLFileModificationDate",
						plist_new_date(st.st_mtime, 0));

				plist_dict_insert_item(dirlist, ent->d_name, fdict);
				free(fpath);
			}
		}
		closedir(cur_dir);
	}
	free(path);

	/* TODO error handling */
	mobilebackup2_error_t err = mobilebackup2_send_status_response(
			mb2s->client, 0, NULL, dirlist);
	plist_free(dirlist);
	if (err != MOBILEBACKUP2_E_SUCCESS) {
		printf("Could not send status response, error %d\n", err);
	}
}

static void mb2_handle_make_directory(mb2_t* mb2s, plist_t message, const char *backup_dir) {
	printf(">> %s called\n", __func__);
	if (!message || (plist_get_node_type(message) != PLIST_ARRAY)
			|| plist_array_get_size(message) < 2 || !backup_dir)
		return;

	plist_t dir = plist_array_get_item(message, 1);
	char *str = NULL;
	int errcode = 0;
	char *errdesc = NULL;
	plist_get_string_val(dir, &str);

	char *newpath = build_path(backup_dir, str, NULL);
	free(str);

	if (mkdir_with_parents(newpath, 0755) < 0) {
		errdesc = strerror(errno);
		if (errno != EEXIST) {
			printf("mkdir: %s (%d)\n", errdesc, errno);
		}
		errcode = errno_to_device_error(errno);
	}
	free(newpath);
	mobilebackup2_error_t err = mobilebackup2_send_status_response(
			mb2s->client, errcode, errdesc, NULL);
	if (err != MOBILEBACKUP2_E_SUCCESS) {
		printf("Could not send status response, error %d\n", err);
	}
}

static void mb2_copy_file_by_path(const char *src, const char *dst) {
	printf(">> %s called\n", __func__);
	FILE *from, *to;
	char buf[BUFSIZ];
	size_t length;

	/* open source file */
	if ((from = fopen(src, "rb")) == NULL) {
		printf("Cannot open source path '%s'.\n", src);
		return;
	}

	/* open destination file */
	if ((to = fopen(dst, "wb")) == NULL) {
		printf("Cannot open destination file '%s'.\n", dst);
		return;
	}

	/* copy the file */
	while ((length = fread(buf, 1, BUFSIZ, from)) != 0) {
		fwrite(buf, 1, length, to);
	}

	if (fclose(from) == EOF) {
		printf("Error closing source file.\n");
	}

	if (fclose(to) == EOF) {
		printf("Error closing destination file.\n");
	}
}

static void mb2_copy_directory_by_path(const char *src, const char *dst) {
	printf(">> %s called\n", __func__);
	if (!src || !dst) {
		return;
	}

	/* if src does not exist */
	struct stat fst;
	if ((stat(src, &fst) != 0) || !S_ISDIR(fst.st_mode)) {
		printf("ERROR: Source directory does not exist '%s': %s (%d)\n", src,
				strerror(errno), errno);
		return;
	}

	/* if dst directory does not exist */
	if ((stat(dst, &fst) != 0) || !S_ISDIR(fst.st_mode)) {
		/* create it */
		if (mkdir_with_parents(dst, 0755) < 0) {
			printf(
					"ERROR: Unable to create destination directory '%s': %s (%d)\n",
					dst, strerror(errno), errno);
			return;
		}
	}

	/* loop over src directory contents */
	DIR *cur_dir = opendir(src);
	if (cur_dir) {
		struct dirent* ent;
		while ((ent = readdir(cur_dir))) {
			char *srcpath = build_path(src, ent->d_name, NULL);
			char *dstpath = build_path(dst, ent->d_name, NULL);
			if (srcpath && dstpath) {
				/* copy file */
				mb2_copy_file_by_path(srcpath, dstpath);

				free(srcpath);
				free(dstpath);
			}
		}
		closedir(cur_dir);
	}
}

/* convert the message string into an integer for switch processing in mb2_process_messages */

static int dlmsg_status_from_string(char *dlmsg) {
	printf(">> %s called\n", __func__);
	if (!strcmp(dlmsg, "DLMessageDownloadFiles"))
		return DLMSG_DOWNLOAD_FILES;
	else if (!strcmp(dlmsg, "DLMessageUploadFiles"))
		return DLMSG_UPLOAD_FILES;
	else if (!strcmp(dlmsg, "DLContentsOfDirectory"))
		return DLMSG_DIRECTORY_CONTENTS;
	else if (!strcmp(dlmsg, "DLMessageCreateDirectory"))
		return DLMSG_CREATE_DIRECTORY;
	else if (!strcmp(dlmsg, "DLMessageMoveFiles"))
		return DLMSG_MOVE_FILES;
	else if (!strcmp(dlmsg, "DLMessageRemoveFiles"))
		return DLMSG_REMOVE_FILES;
	else if (!strcmp(dlmsg, "DLMessageCopyItem"))
		return DLMSG_COPY_ITEM;
	else if (!strcmp(dlmsg, "DLMessageDisconnect"))
		return DLMSG_DISCONNECT;
	else if (!strcmp(dlmsg, "DLMessageProcessMessage"))
		return DLMSG_PROCESS_MESSAGE;
	else
		return 0;
}

/* we process the messages here that instruct what to do while restoring or backing up */

static int mb2_process_messages(mb2_t* mb2, const char* backup_directory) {
	printf(">> %s called\n", __func__);
	plist_t node_tmp = NULL;
	mobilebackup2_error_t err;

	/* reset operation success status */
	int operation_ok = 0;
	plist_t message = NULL;

	char *dlmsg = NULL;
	int file_count = 0;
	int errcode = 0;
	const char *errdesc = NULL;

	do {
		if (dlmsg) {
			free(dlmsg);
			dlmsg = NULL;
		}
		mobilebackup2_receive_message(mb2->client, &message, &dlmsg);
		if (!message || !dlmsg) {
			if (message)
				free(message);
			if (dlmsg)
				free(dlmsg);
			if (mb2->poison_spilled) {
				return 0xDEAD;
			}
			printf(
					"Device is not ready yet. Going to try again in 2 seconds...\n");

			//their old code wasn't trying again, not sure how we would either, just returning for now.
			sleep(2);
			return -1;
		}

		int dlStatus = dlmsg_status_from_string(dlmsg);

		switch (dlStatus) { //int value of DLMessage

		uint32_t cnt = 0;

	case DLMSG_DOWNLOAD_FILES:

		/* device wants to download files from the computer */
		mb2_handle_send_files(mb2, message, backup_directory);

		break;

	case DLMSG_UPLOAD_FILES:

		/* device wants to send files to the computer */
		file_count += mb2_handle_receive_files(mb2, message, backup_directory);

		break;

	case DLMSG_DIRECTORY_CONTENTS:

		/* list directory contents */
		mb2_handle_list_directory(mb2, message, backup_directory);

		break;

	case DLMSG_CREATE_DIRECTORY:

		/* make a directory */
		mb2_handle_make_directory(mb2, message, backup_directory);

		break;

	case DLMSG_MOVE_FILES:

		/* perform a series of rename operations */
		errcode = 0;
		plist_t moves = plist_array_get_item(message, 1);
		cnt = plist_dict_get_size(moves);
		printf("Moving %d file%s\n", cnt, (cnt == 1) ? "" : "s");
		plist_dict_iter iter = NULL;
		plist_dict_new_iter(moves, &iter);

		errdesc = NULL;
		if (iter) {

			char *key = NULL;
			plist_t val = NULL;
			do {
				plist_dict_next_item(moves, iter, &key, &val);
				if (key && (plist_get_node_type(val) == PLIST_STRING)) {
					char *str = NULL;
					plist_get_string_val(val, &str);
					if (str) {
						char *newpath = build_path(backup_directory, str, NULL);
						free(str);
						char *oldpath = build_path(backup_directory, key, NULL);

						remove(newpath);
						if (rename(oldpath, newpath) < 0) {
							printf("Renaming '%s' to '%s' failed: %s (%d)\n",
									oldpath, newpath, strerror(errno), errno);
							errcode = errno_to_device_error(errno);
							errdesc = strerror(errno);
							break;
						}
						free(oldpath);
						free(newpath);
					}
					free(key);
					key = NULL;
				}
			} while (val);
			free(iter);
		} else {
			errcode = -1;
			errdesc = "Could not create dict iterator";
			printf("Could not create dict iterator\n");
		}
		err = mobilebackup2_send_status_response(mb2->client, errcode, errdesc,
				plist_new_dict());
		if (err != MOBILEBACKUP2_E_SUCCESS) {
			printf("Could not send status response, error %d\n", err);
		}

		break;

	case DLMSG_REMOVE_FILES:

		errcode = 0;
		plist_t removes = plist_array_get_item(message, 1);
		cnt = plist_array_get_size(removes);
		printf("Removing %d file%s\n", cnt, (cnt == 1) ? "" : "s");
		uint32_t ii = 0;
		errdesc = NULL;
		for (ii = 0; ii < cnt; ii++) {
			plist_t val = plist_array_get_item(removes, ii);
			if (plist_get_node_type(val) == PLIST_STRING) {
				char *str = NULL;
				plist_get_string_val(val, &str);
				if (str) {
					char *newpath = build_path(backup_directory, str, NULL);
					free(str);
					if (remove(newpath) < 0) {
						printf("Could not remove '%s': %s (%d)\n", newpath,
								strerror(errno), errno);
						errcode = errno_to_device_error(errno);
						errdesc = strerror(errno);
					}
					free(newpath);
				}
			}
		}
		err = mobilebackup2_send_status_response(mb2->client, errcode, errdesc,
				plist_new_dict());
		if (err != MOBILEBACKUP2_E_SUCCESS) {
			printf("Could not send status response, error %d\n", err);
		}

		break;

	case DLMSG_COPY_ITEM:

		errcode = 0;
		plist_t srcpath = plist_array_get_item(message, 1);
		plist_t dstpath = plist_array_get_item(message, 2);

		errdesc = NULL;
		if ((plist_get_node_type(srcpath) == PLIST_STRING)
				&& (plist_get_node_type(dstpath) == PLIST_STRING)) {
			char *src = NULL;
			char *dst = NULL;
			plist_get_string_val(srcpath, &src);
			plist_get_string_val(dstpath, &dst);
			if (src && dst) {
				char *oldpath = build_path(backup_directory, src, NULL);
				char *newpath = build_path(backup_directory, dst, NULL);

				printf("Copying '%s' to '%s'\n", src, dst);

				/* check that src exists */
				struct stat fst;
				if (stat(oldpath, &fst) == 0) {
					if (S_ISDIR(fst.st_mode)) {
						mb2_copy_directory_by_path(oldpath, newpath);
					} else if (S_ISREG(fst.st_mode)) {
						mb2_copy_file_by_path(oldpath, newpath);
					}
				}

				free(newpath);
				free(oldpath);
			}
			free(src);
			free(dst);
		}

		err = mobilebackup2_send_status_response(mb2->client, errcode, errdesc,
				plist_new_dict());
		if (err != MOBILEBACKUP2_E_SUCCESS) {
			printf("Could not send status response, error %d\n", err);
		}

		break;

	case DLMSG_DISCONNECT:

		//guess we do nothing here?
		break;

	case DLMSG_PROCESS_MESSAGE:

		node_tmp = plist_array_get_item(message, 1);
		if (plist_get_node_type(node_tmp) != PLIST_DICT) {
			printf("Unknown message received!\n");
		}
		plist_t nn;
		int error_code = -1;
		nn = plist_dict_get_item(node_tmp, "ErrorCode");
		if (nn && (plist_get_node_type(nn) == PLIST_UINT)) {
			uint64_t ec = 0;
			plist_get_uint_val(nn, &ec);
			error_code = (uint32_t) ec;
			if (error_code == 0) {
				operation_ok = 1;
			}
		}
		nn = plist_dict_get_item(node_tmp, "ErrorDescription");
		char *str = NULL;
		if (nn && (plist_get_node_type(nn) == PLIST_STRING)) {
			plist_get_string_val(nn, &str);
		}
		if (error_code != 0) {
			if (str) {
				printf("ErrorCode %d: %s\n", error_code, str);
			} else {
				printf("ErrorCode %d: (Unknown)\n", error_code);
			}
		}
		if (str) {
			free(str);
		}
		nn = plist_dict_get_item(node_tmp, "Content");
		if (nn && (plist_get_node_type(nn) == PLIST_STRING)) {
			str = NULL;
			plist_get_string_val(nn, &str);
			printf("Content:\n");
			printf("%s", str);
			free(str);
		}

		break;

	default:

		//nada
		break;
		}

	} while (1);

	return errcode; //if we got this far, success?
}

#define DUMMYBACKUPDIR "nirvana"

// Implement these for jailbreak
int mb2_crash(mb2_t* mb2) {
	char *poison_buf = "___EmptyParameterString___";
	int poison_length = strlen(poison_buf);
	/* send Hello message */
	double local_versions[2] = { 2.0, 2.1 };
	double remote_version = 0.0;
	mobilebackup2_error_t err = mobilebackup2_version_exchange(mb2->client, local_versions, 2, &remote_version);
	if (err != MOBILEBACKUP2_E_SUCCESS) {
		printf("Could not perform backup protocol version exchange, error code %d\n", err);
		return -1;
	}

	printf("Negotiated Protocol Version %.1f\n", remote_version);

	printf("Starting *special* restore...\n");

	mkdir_with_parents(DUMMYBACKUPDIR, 0755);

	/* make sure backup device sub-directory exists */
	char *devbackupdir = build_path(DUMMYBACKUPDIR, mb2->device->uuid, NULL);
	mkdir_with_parents(devbackupdir, 0755);

	char *statusplist = build_path(devbackupdir, "Status.plist", NULL);
	free(devbackupdir);

	plist_t stpl = plist_new_dict();
	plist_dict_insert_item(stpl, "UUID", plist_new_string("whatever"));
	plist_dict_insert_item(stpl, "IsFullBackup", plist_new_bool(0));
	plist_dict_insert_item(stpl, "Version", plist_new_string("2.4"));
	plist_dict_insert_item(stpl, "BackupState", plist_new_string("new"));
	plist_dict_insert_item(stpl, "Date", plist_new_date(time(NULL), 0));

	plist_dict_insert_item(stpl, "SnapshotState", plist_new_string("finished"));
	plist_write_to_filename(stpl, statusplist, PLIST_FORMAT_BINARY);
	plist_free(stpl);
	free(statusplist);

	err = mobilebackup2_send_request(mb2->client, "Restore", mb2->device->uuid, mb2->device->uuid, NULL);
	if (err == MOBILEBACKUP2_E_SUCCESS) {
		// enable crashing in mb2_handle_send_files()

		mb2->poison = poison_buf;
		mb2->poison_length = poison_length;
	} else {
		if (err == MOBILEBACKUP2_E_BAD_VERSION) {
			printf("ERROR: Could not start backup process: backup protocol version mismatch!\n");
		} else if (err == MOBILEBACKUP2_E_REPLY_NOT_OK) {
			printf("ERROR: Could not start backup process: device refused to start the backup process.\n");
		} else {
			printf("ERROR: Could not start backup process: unspecified error occured\n");
		}
	}

	int processStatus = mb2_process_messages(mb2, DUMMYBACKUPDIR);

	if (processStatus == 0xDEAD) {
		return 0;
	} else {
		return -1;
	}
}

int mb2_inject(mb2_t* mb2, char* data, int size) {
	double local_versions[2] = { 2.0, 2.1 };
	double remote_version = 0.0;
	mobilebackup2_error_t err = mobilebackup2_version_exchange(mb2->client, local_versions, 2, &remote_version);
	if (err != MOBILEBACKUP2_E_SUCCESS) {
		printf("Could not perform backup protocol version exchange, error code %d\n", err);
		return -1;
	}

	printf("Negotiated Protocol Version %.1f\n", remote_version);

	printf("Starting backup...\n");

	mkdir_with_parents(DUMMYBACKUPDIR, 0755);

	/* make sure backup device sub-directory exists */
	char *devbackupdir = build_path(DUMMYBACKUPDIR, mb2->device->uuid, NULL);
	mkdir_with_parents(devbackupdir, 0755);

	char *statusplist = build_path(devbackupdir, "Status.plist", NULL);
	free(devbackupdir);

	plist_t stpl = plist_new_dict();
	plist_dict_insert_item(stpl, "UUID", plist_new_string("whatever"));
	plist_dict_insert_item(stpl, "IsFullBackup", plist_new_bool(0));
	plist_dict_insert_item(stpl, "Version", plist_new_string("2.4"));
	plist_dict_insert_item(stpl, "BackupState", plist_new_string("new"));
	plist_dict_insert_item(stpl, "Date", plist_new_date(time(NULL), 0));

	plist_dict_insert_item(stpl, "SnapshotState", plist_new_string("finished"));
	plist_write_to_filename(stpl, statusplist, PLIST_FORMAT_BINARY);
	plist_free(stpl);
	free(statusplist);

	char* poison_buf = data;
	int poison_length = size;

	err = mobilebackup2_send_request(mb2->client, "Backup", mb2->device->uuid, NULL, NULL);
	if (err == MOBILEBACKUP2_E_SUCCESS) {
		// enable crashing in mb2_handle_send_files()

		mb2->poison = poison_buf;
		mb2->poison_length = poison_length;
	} else {
		if (err == MOBILEBACKUP2_E_BAD_VERSION) {
			printf("ERROR: Could not start backup process: backup protocol version mismatch!\n");
		} else if (err == MOBILEBACKUP2_E_REPLY_NOT_OK) {
			printf("ERROR: Could not start backup process: device refused to start the backup process.\n");
		} else {
			printf("ERROR: Could not start backup process: unspecified error occured\n");
		}
	}

	int processStatus = mb2_process_messages(mb2, DUMMYBACKUPDIR);

	if (processStatus == 0xDEAD) {
		return 0;
	} else {
		return -1;
	}
}

int mb2_exploit(mb2_t* mb2) {
	return 0;
}

void mb2_set_status_plist_cb_func(mb2_status_plist_cb_t callback, void *userdata) {
	status_cb = callback;
	status_cb_userdata = NULL;
}

void mb2_set_attack_plist_cb_func(mb2_attack_plist_cb_t callback, void *userdata) {
	attack_cb = callback;
	attack_cb_userdata = NULL;
}

