AM_CFLAGS =	                   \
	$(GLOBAL_CFLAGS)           \
	$(libimobiledevice_CFLAGS) \
	$(libplist_CFLAGS)         \
	$(libcrypto_CFLAGS)        

AM_LDFLAGS =                   \
	$(GLOBAL_LIBS)             \
	$(libimobiledevice_LIBS)   \
	$(libplist_LIBS)           \
	$(libcrypto_LIBS)          

bin_PROGRAMS = absinthe

absinthe_SOURCES = absinthe.c device.c lockdown.c mb2.c afc.c crashreporter.c crashreportmover.c crashreportcopy.c crashreport.c
absinthe_CFLAGS = $(AM_CFLAGS)
absinthe_LDFLAGS = $(AM_LDFLAGS)