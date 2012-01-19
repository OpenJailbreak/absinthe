AM_CFLAGS = $(GLOBAL_CFLAGS) $(GUI_CFLAGS)
AM_LDFLAGS = $(GLOBAL_LDFLAGS) $(GUI_LDFLAGS)
AM_LDADD = $(GLOBAL_LIBS) $(GUI_LDADD)

bin_PROGRAMS = absinthe

absinthe_SOURCES = \
	main.cpp \
	AbsintheMainWnd.cpp \
	AbsintheWorker.cpp \
	AbsintheJailbreaker.cpp \
	../src/jailbreak.c \
	../src/common.c \
	../src/file.c \
	../src/backup.c \
	../src/backup_file.c \
	../src/mbdb.c \
	../src/mbdb_record.c \
	../src/device.c \
	../src/lockdown.c \
	../src/mb1.c \
	../src/afc.c \
	../src/plist_extras.c \
	../src/crashreporter.c \
	../src/crashreportmover.c \
	../src/crashreportcopy.c \
	../src/crashreport.c \
	../src/idevicebackup2.c \
	../src/fsgen.c 

absinthe_CFLAGS = $(AM_CFLAGS) -I../src
absinthe_CXXFLAGS = $(AM_CFLAGS) -I../src
absinthe_LDFLAGS = $(AM_LDFLAGS)
absinthe_LDADD = $(AM_LDADD)
