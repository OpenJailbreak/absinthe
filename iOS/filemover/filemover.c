#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <syslog.h>

#define MOVE_DIR "/private/var/mobile/Media/HackStore"
#define ORIG_DIR "/private/var/mobile/Media"

typedef struct kinfo_proc kinfo_proc;

static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount) /*{{{*/
{
	int err;
	kinfo_proc* result;
	bool done;
	static const int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
	size_t length;

	assert(procList != NULL);
	assert(*procList == NULL);
	assert(procCount != NULL);

	*procCount = 0;

	result = NULL;
	done = false;
	do {
		assert(result == NULL);

		length = 0;
		err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1, NULL, &length, NULL, 0);
		if (err == -1) {
			err = errno;
		}

		if (err == 0) {
			result = malloc(length);
			if (result == NULL) {
				err = ENOMEM;
			}
		}

		if (err == 0) {
			err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1, result, &length, NULL, 0);
			if (err == -1) {
				err = errno;
			}
			if (err == 0) {
				done = true;
			} else if (err == ENOMEM) {
				assert(result != NULL);
				free(result);
				result = NULL;
				err = 0;
			}
		}
	} while (err == 0 && ! done);

	if (err != 0 && result != NULL) {
		free(result);
		result = NULL;
	}
	*procList = result;
	if (err == 0) {
		*procCount = length / sizeof(kinfo_proc);
	}

	assert( (err == 0) == (*procList != NULL) );

	return err;
} /*}}}*/

/* recursively remove path, including path */
static void rmdir_recursive(const char *path) /*{{{*/
{
	if (!path) {
		return;
	}
	DIR* cur_dir = opendir(path);
	if (cur_dir) {
		struct dirent* ep;
		while ((ep = readdir(cur_dir))) {
			if ((strcmp(ep->d_name, ".") == 0) || (strcmp(ep->d_name, "..") == 0)) {
				continue;
			}
			char *fpath = (char*)malloc(strlen(path)+1+strlen(ep->d_name)+1);
			if (fpath) {
				struct stat st;
				strcpy(fpath, path);
				strcat(fpath, "/");
				strcat(fpath, ep->d_name);

				if ((stat(fpath, &st) == 0) && S_ISDIR(st.st_mode)) {
					rmdir_recursive(fpath);
				} else {
					if (remove(fpath) != 0) {
						//syslog(LOG_NOTICE, "could not remove %s: %s\n", fpath, strerror(errno));
					}
				}
				free(fpath);
			}
		}
		closedir(cur_dir);
	}
	if (rmdir(path) != 0) {
		//syslog(LOG_NOTICE, "could not remove %s: %s\n", path, strerror(errno));
	}
} /*}}}*/

void sig_chld_ignore(int signal)
{
    return;
}

void sig_chld_waitpid(int signal)
{
    while(waitpid(-1, 0, WNOHANG) > 0);
}

void executeCommand(uid_t uid, gid_t gid, char *const argv[])
{
    signal(SIGCHLD, &sig_chld_ignore);

    pid_t fork_pid;
    if((fork_pid = fork()) != 0)
    {
        while(waitpid(fork_pid, NULL, WNOHANG) <= 0)
            usleep(300);
    } else
    {
        if(uid == 501)
            chdir("/private/var/mobile");

        setuid(uid);
        setgid(gid);

        if(execv(argv[0], argv) != 0)
        {
            perror("execv");
            fflush(stderr);
            fflush(stdout);
        }
        exit(0);
    }
    signal(SIGCHLD, &sig_chld_waitpid);
}

static void start()
{
	syslog(LOG_NOTICE, "waiting for springboard to launch...\n");
	int found = 0;
	while (1) {
		size_t proc_count = 0;
		kinfo_proc *proc_list = NULL;
		GetBSDProcessList(&proc_list, &proc_count);
		int i;
		for (i = 0; i < proc_count; i++) {
			if (!strcmp((&proc_list[i])->kp_proc.p_comm, "SpringBoard")) {
				found = 1;
			}
		}
		free(proc_list);
		if (found) {
			break;
		}
		sleep(3);
	}
	syslog(LOG_NOTICE, "Springboard started. Moving back files...\n");

	DIR* dir = opendir(MOVE_DIR);
	if (dir) {
		struct dirent* ep = NULL;
		while ((ep = readdir(dir))) {
			if (!ep->d_name) continue;
			if (!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, "..")) continue;
			char* oldname = (char*)malloc(strlen(MOVE_DIR)+1+strlen(ep->d_name)+1);
			strcpy(oldname, MOVE_DIR);
			strcat(oldname, "/");
			strcat(oldname, ep->d_name);

			char* newname = (char*)malloc(strlen(ORIG_DIR)+1+strlen(ep->d_name)+1);
			strcpy(newname, ORIG_DIR);
			strcat(newname, "/");
			strcat(newname, ep->d_name);

			// make sure the directory in the target dir is removed
			rmdir_recursive(newname);

			// and move directory
			syslog(LOG_NOTICE, "moving %s to %s\n", oldname, newname);
			rename(oldname, newname);

			free(newname);
			free(oldname);
		}
		closedir(dir);
		remove(MOVE_DIR);
		dir = opendir(MOVE_DIR);
		if (dir) {
			closedir(dir);
			syslog(LOG_ERR, "WARNING: the folder '"MOVE_DIR"' is still present in the user's Media folder. You have to check yourself for any leftovers and move them back if required!");
		}
	}

//        syslog(LOG_NOTICE, "Executing uicache...\n");

//        executeCommand(501, 501, (char* const []) {"/usr/bin/uicache", NULL});

	syslog(LOG_NOTICE, "Done. Removing myself.\n");
	system("launchctl unload /Library/LaunchDaemons/com.chronic-dev.greenpois0n.corona.filemover.plist");
	remove("/Library/LaunchDaemons/com.chronic-dev.greenpois0n.corona.filemover.plist");
	remove("/private/var/root/filemover");
	syslog(LOG_NOTICE, "Bye.\n");
}

int main(int argc, char** argv)
{
	pid_t pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	pid_t sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}
	close(STDIN_FILENO);
	setuid(0);

	char* name = strrchr(argv[0], '/');
	name = (name ? name+1 : argv[0]);
	openlog(name, LOG_PID, 0);

	start();

	closelog();

	return 0;
}
