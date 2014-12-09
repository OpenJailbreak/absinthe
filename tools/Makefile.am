AM_CFLAGS = $(GLOBAL_CFLAGS) -g -I. -I../src
AM_LDFLAGS = $(GLOBAL_LDFLAGS)
AM_LDADD = $(GLOBAL_LIBS)

bin_PROGRAMS = gizmo dyldrop pointingdb machoman mb2insert fsgen decache binpatch mbdbtool handoff injectconf

gizmo_SOURCES = gizmo.c
gizmo_CFLAGS = $(AM_CFLAGS)
gizmo_LDFLAGS = $(AM_LDFLAGS)
gizmo_LDADD = $(AM_LDADD)

dyldrop_SOURCES = dyldrop.c \
				../src/common.c \
				../src/file.c \
				../src/dyldcache.c \
				../src/dyldimage.c \
				../src/dyldmap.c \
				../src/macho.c \
				../src/macho_command.c \
				../src/macho_segment.c \
				../src/macho_section.c \
				../src/macho_symbol.c \
				../src/macho_symtab.c
dyldrop_CFLAGS = $(AM_CFLAGS)
dyldrop_LDFLAGS = $(AM_LDFLAGS)
dyldrop_LDADD = $(AM_LDADD)

pointingdb_SOURCES = pointingdb.c \
				../src/common.c \
				../src/file.c
pointingdb_CFLAGS = $(AM_CFLAGS)
pointingdb_LDFLAGS = $(AM_LDFLAGS)
pointingdb_LDADD = $(AM_LDADD)

machoman_SOURCES = machoman.c \
				../src/common.c \
				../src/file.c \
				../src/macho.c \
				../src/macho_command.c \
				../src/macho_segment.c \
				../src/macho_section.c \
				../src/macho_symbol.c \
				../src/macho_symtab.c
machoman_CFLAGS = $(AM_CFLAGS)
machoman_LDFLAGS = $(AM_LDFLAGS)
machoman_LDADD = $(AM_LDADD)

mb2insert_SOURCES = mb2insert.c \
				../src/idevicebackup2.c \
				rop.c \
				../src/file.c \
				../src/common.c \
				../src/backup.c \
				../src/backup_file.c \
				../src/mbdb.c \
				../src/mbdb_record.c 
mb2insert_CFLAGS = $(AM_CFLAGS)
mb2insert_LDFLAGS = $(AM_LDFLAGS)
mb2insert_LDADD = $(AM_LDADD)

fsgen_SOURCES = fsgen.c rop.c
fsgen_CFLAGS = $(AM_CFLAGS)
fsgen_LDFLAGS = $(AM_LDFLAGS)
fsgen_LDADD = $(AM_LDADD)

decache_SOURCES = \
				decache.c \
				../src/file.c \
				../src/common.c \
				../src/dyldcache.c \
				../src/dyldimage.c \
				../src/dyldmap.c
				
decache_CFLAGS = $(AM_CFLAGS)
decache_LDFLAGS = $(AM_LDFLAGS)
decache_LDADD = $(AM_LDADD)

binpatch_SOURCES = \
				binpatch.c \
				../src/file.c \
				../src/common.c \
				../src/bpatch.c
				
binpatch_CFLAGS = $(AM_CFLAGS)
binpatch_LDFLAGS = $(AM_LDFLAGS)
binpatch_LDADD = $(AM_LDADD)

handoff_SOURCES = \
				handoff.c \
				../src/device.c \
				../src/file.c \
				../src/common.c 
				
handoff_CFLAGS = $(AM_CFLAGS)
handoff_LDFLAGS = $(AM_LDFLAGS)
handoff_LDADD = $(AM_LDADD)

switchapp_CFLAGS = $(AM_CFLAGS)
switchapp_LDFLAGS = $(AM_LDFLAGS)
switchapp_LDADD = $(AM_LDADD)

switchapp_SOURCES = \
				switchapp.c \
				../src/device.c \
				../src/common.c \
				../src/idevicebackup2.c
				
injectconf_CFLAGS = $(AM_CFLAGS)
injectconf_LDFLAGS = $(AM_LDFLAGS)
injectconf_LDADD = $(AM_LDADD)

injectconf_SOURCES = \
				injectconf.c \
				../src/device.c \
				../src/common.c \
				../src/idevicebackup2.c \
				../src/backup.c \
				../src/backup_file.c \
				../src/plist_extras.c \
				../src/file.c \
				../src/mbdb.c \
				../src/mbdb_record.c
				
mbdbtool_CFLAGS = $(AM_CFLAGS)
mbdbtool_LDFLAGS = $(AM_LDFLAGS)
mbdbtool_LDADD = $(AM_LDADD)

mbdbtool_SOURCES = \
				mbdbtool.c \
				../src/device.c \
				../src/common.c \
				../src/idevicebackup2.c \
				../src/backup.c \
				../src/backup_file.c \
				../src/plist_extras.c \
				../src/file.c \
				../src/mbdb.c \
				../src/mbdb_record.c