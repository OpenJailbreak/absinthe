AM_CFLAGS = $(GLOBAL_CFLAGS) $(GUI_CFLAGS)
AM_CXXFLAGS = $(AM_CFLAGS)
AM_LDFLAGS = $(GLOBAL_LDFLAGS) $(GUI_LDFLAGS)
AM_LDADD = $(GLOBAL_LIBS) $(GUI_LDADD)
if LINUX
AM_GUI_SRC_FILES=AbsintheMainWnd_linux.cpp
endif
if OSX
AM_GUI_SRC_FILES=AbsintheMainWnd_osx.cpp
AM_CXXFLAGS += -ObjC++
endif
if WIN32
AM_GUI_SRC_FILES=AbsintheMainWnd_win32.cpp
endif

bin_PROGRAMS = absinthe

absinthe_SOURCES = \
	main.cpp \
	$(AM_GUI_SRC_FILES) \
	AbsintheWorker.cpp \
	AbsintheJailbreaker.cpp \
	iTunesKiller.cpp \
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
	../src/fsgen.c \
	../src/bpatch.c

absinthe_CFLAGS = $(AM_CFLAGS) -I../src
absinthe_CXXFLAGS = $(AM_CXXFLAGS) -I../src
absinthe_LDFLAGS = $(AM_LDFLAGS)
absinthe_LDADD = $(AM_LDADD)
