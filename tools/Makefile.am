AM_CFLAGS = $(GLOBAL_CFLAGS)
AM_LDFLAGS = $(GLOBAL_LDFLAGS)
AM_LDADD = $(GLOBAL_LIBS)

bin_PROGRAMS = gizmo dyldrop

gizmo_SOURCES = gizmo.c

gizmo_CFLAGS = $(AM_CFLAGS) -I../src
gizmo_LDFLAGS = $(AM_LDFLAGS)
gizmo_LDADD = $(AM_LDADD)

dyldrop_SOURCES = dyldrop.c ../src/common.c ../src/file.c ../src/dyldcache.c ../src/dyldimage.c ../src/dyldmap.c ../src/macho.c ../src/macho_command.c ../src/macho_symtab.c ../src/macho_segment.c ../src/macho_section.c ../src/macho_symbol.c

dyldrop_CFLAGS = $(AM_CFLAGS) -I../src
dyldrop_LDFLAGS = $(AM_LDFLAGS)
dyldrop_LDADD = $(AM_LDADD)
