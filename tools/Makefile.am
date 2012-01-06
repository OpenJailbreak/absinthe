AM_CFLAGS = $(GLOBAL_CFLAGS)
AM_LDFLAGS = $(GLOBAL_LDFLAGS)
AM_LDADD = $(GLOBAL_LIBS)

bin_PROGRAMS = gizmo dyldrop pointingdb machoman mb2insert

gizmo_SOURCES = gizmo.c
gizmo_CFLAGS = $(AM_CFLAGS) -I../src
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
dyldrop_CFLAGS = $(AM_CFLAGS) -I../src
dyldrop_LDFLAGS = $(AM_LDFLAGS)
dyldrop_LDADD = $(AM_LDADD)

pointingdb_SOURCES = pointingdb.c \
				../src/common.c \
				../src/file.c
pointingdb_CFLAGS = $(AM_CFLAGS) -I../src
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
machoman_CFLAGS = $(AM_CFLAGS) -I../src
machoman_LDFLAGS = $(AM_LDFLAGS)
machoman_LDADD = $(AM_LDADD)

mb2insert_SOURCES = mb2insert.c \
				idevicebackup2.c \
				../src/file.c \
				../src/backup.c \
				../src/backup_file.c \
				../src/mbdb.c \
				../src/mbdb_record.c
mb2insert_CFLAGS = $(AM_CFLAGS) -I. -I../src
mb2insert_LDFLAGS = $(AM_LDFLAGS)
mb2insert_LDADD = $(AM_LDADD)

