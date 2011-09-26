dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(absinthe, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AM_PROG_CC_C_O
AC_CONFIG_HEADERS(config.h)

ACCFLAGS="-m32 -g "
ACSHAREDLIBS=""

SYSTYPE=
case "$host_os" in
	darwin*)
		SYSTYPE=osx
		ACCFLAGS+="-fPIC -arch i386 "
		ACSHAREDLIBS+="-lpthread -lz -lm "
	;;
	linux*)
		SYSTYPE=linux
		ACCFLAGS+="-fPIC "
		ACSHAREDLIBS+="-lpthread -lz -lm "
	;;
	*)
	;;
esac

if test "x$SYSTYPE" == "x"; then
	AC_MSG_ERROR([Unsupported system type $UNAME])
fi

GLOBAL_CFLAGS=$ACCFLAGS
AC_SUBST(GLOBAL_CFLAGS)

LIBDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
GLOBAL_LIBS=" $ACSHAREDLIBS \
 $LIBDIR/libimobiledevice.a \
 $LIBDIR/libplist.a \
 $LIBDIR/libxml2.a \
 $LIBDIR/libssl.a \
 $LIBDIR/libcrypto.a \
 $LIBDIR/libusbmuxd.a \
"
AC_SUBST(GLOBAL_LIBS)

GLOBAL_LDFLAGS="-static-libgcc"
AC_SUBST(GLOBAL_LDFLAGS)

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

