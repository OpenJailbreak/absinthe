dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(absinthe, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AM_PROG_CC_C_O
AC_CONFIG_HEADERS(config.h)

# Comment this out for production build to remove all debugging strings
AC_DEFINE(_DEBUG,1,[debugging])

ACCFLAGS="-m32 -I\$(top_srcdir)/include -DLIBXML_STATIC=1 -g "
ACSHAREDLIBS=""

SYSTYPE=
case "$host_os" in
	darwin*)
		SYSTYPE=osx
		ACCFLAGS+="-fPIC -arch i386 "
		ACSHAREDLIBS+="-lpthread -lz -lm -liconv "
	;;
	linux*)
		SYSTYPE=linux
		ACCFLAGS+="-fPIC "
		ACSHAREDLIBS+="-lpthread -lz -lm "
	;;
	mingw32)
		SYSTYPE=win32
		ACCFLAGS+="-DWIN32 "
		ACSHAREDLIBS+="-lz -lm -lws2_32 -lgdi32 -lole32 "
	;;
	*)
	;;
esac

if test "x$SYSTYPE" == "x"; then
	AC_MSG_ERROR([Unsupported system type $host_os])
fi

GLOBAL_CFLAGS=$ACCFLAGS
AC_SUBST(GLOBAL_CFLAGS)

LIBDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
GLOBAL_LIBS=" \
 $LIBDIR/libimobiledevice.a \
 $LIBDIR/libplist.a \
 $LIBDIR/libxml2.a \
 $LIBDIR/libssl.a \
 $LIBDIR/libcrypto.a \
 $LIBDIR/libusbmuxd.a \
 $ACSHAREDLIBS \ 
"
AC_SUBST(GLOBAL_LIBS)

GLOBAL_LDFLAGS="-static-libgcc"
AC_SUBST(GLOBAL_LDFLAGS)

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

