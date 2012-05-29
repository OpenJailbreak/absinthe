dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(absinthe, 1.0)


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AM_PROG_CC_C_O
AC_CONFIG_HEADERS(config.h)

# Comment this out for production build to remove all debugging strings
AC_DEFINE(_DEBUG,1,[debugging])

AC_PROG_CXX

ACCFLAGS="-m32 -I\$(top_srcdir)/include -g -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES "
ACSHAREDLIBS=""
ACLDFLAGS=""

LIBXML2_STATIC=""
OPENSSL_STATIC=""

SYSTYPE=
case "$host_os" in
	darwin*)
		if test -d "/Applications/Xcode.app/Contents/Developer"; then
			SDK_DIR=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
		else
			SDK_DIR=/Developer/SDKs
		fi
		if test "x$BUILD_10_5" = "x1"; then
		SYSTYPE=osx.10.5
		ACCFLAGS+="-fPIC -arch i386 -arch ppc -isysroot $SDK_DIR/MacOSX10.5.sdk -mmacosx-version-min=10.5 "
		ACSHAREDLIBS+="-lpthread -lz -lm -liconv -lssl -lcrypto -lxml2 "
		ACLDFLAGS="-arch i386 -arch ppc "
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		ACGUI_CFLAGS+="-DPIC -DHAVE_ASPRINTF -DHAVE_VASPRINTF \ 
                        -DCURL_PULL_SYS_SOCKET_H \
                        -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES"
                ACGUI_LDFLAGS+="-pthread -static-libstdc++"
                ACGUI_LDADD+="-framework IOKit -framework Cocoa -framework System"
		else
		SYSTYPE=osx
		ACCFLAGS+="-fPIC -arch i386 "
		ACSHAREDLIBS+="-lpthread -lz -lm -liconv -lssl -lcrypto -lxml2 "
		ACLDFLAGS+="-arch i386 "
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		ACGUI_CFLAGS+="-DPIC -DHAVE_ASPRINTF -DHAVE_VASPRINTF \
			-isysroot $SDK_DIR/MacOSX10.6.sdk \
			-DCURL_PULL_SYS_SOCKET_H \
			-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES"
		ACGUI_LDFLAGS+="-pthread -static-libstdc++"
		ACGUI_LDADD+="-framework IOKit -framework Cocoa -framework System -framework Security"
		fi
	;;
	linux*)
		PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= 2.16)
		PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16)
		case "$BUILD_HOST_CPU" in
		x86_64)
		SYSTYPE=linux.x86_64
		ACCFLAGS="-fPIC -I\$(top_srcdir)/include -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES "
		ACSHAREDLIBS+="-lpthread -lz -lm -lssl -lcrypto -lxml2"
		ACLDFLAGS+="-s"
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		ACGUI_CFLAGS+="-DPIC -DHAVE_ASPRINTF -DHAVE_VASPRINTF \
			\$(GTK2_CFLAGS) \$(GTHREAD_CFLAGS) -pthread"
                ACGUI_LDFLAGS+="-pthread -Wl,-Bsymbolic-functions -static-libstdc++"
                ACGUI_LDADD+="-lpthread -lz -lm \$(GTK2_LIBS) \$(GTHREAD_LIBS)"
		;;
		*)
		SYSTYPE=linux
		ACCFLAGS+="-fPIC -I\$(top_srcdir)/include -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES "
		ACSHAREDLIBS+="-lpthread -lz -lm -lssl -lcrypto -lxml2"
		ACLDFLAGS+="-s"
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		ACGUI_CFLAGS+="-DPIC -DHAVE_ASPRINTF -DHAVE_VASPRINTF \
			\$(GTK2_CFLAGS) \$(GTHREAD_CFLAGS) -pthread"
		ACGUI_LDFLAGS+="-pthread -Wl,-Bsymbolic-functions -L/lib32 -L/usr/lib32 -static-libstdc++"
		ACGUI_LDADD+="-lpthread -lz -lm \$(GTK2_LIBS) \$(GTHREAD_LIBS)"
		;;
		esac
	;;
	mingw32)
		SYSTYPE=win32
		ACCFLAGS+="-DWIN32 -D__LITTLE_ENDIAN__=1 -DLIBXML_STATIC=1 "
		ACSHAREDLIBS+="\$(top_srcdir)/staticlibs/win32/libz.a -lm -lws2_32 -lgdi32 -lole32 "
		ACLDFLAGS+="-s"
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		ACGUI_CFLAGS+="-D_WIN32 -D__LITTLE_ENDIAN__=1 \
			-DCURL_PULL_WS2TCPIP_H=1 -mthreads"
		ACGUI_LDFLAGS+="-mthreads -Wl,--subsystem,windows -mwindows -static-libstdc++"
		ACGUI_LDADD+="\$(top_srcdir)/gui/win32res.o \
			-lm -lws2_32 -lgdi32 -lole32 -loleaut32 -luuid -lcomctl32 -lsetupapi"
		LIBXML2_STATIC="$LIBSDIR/libxml2.a"
		OPENSSL_STATIC="$LIBSDIR/libssl.a $LIBSDIR/libcrypto.a"
	;;
	*)
	;;
esac

if test "x$SYSTYPE" == "x"; then
	AC_MSG_ERROR([Unsupported system type $host_os])
fi

AM_CONDITIONAL([LINUX], [test x$SYSTYPE = xlinux] || [test x$SYSTYPE = xlinux.x86_64])
AM_CONDITIONAL([OSX], [test x$SYSTYPE = xosx] || [test x$SYSTYPE = xosx.10.5])
AM_CONDITIONAL([WIN32], [test x$SYSTYPE = xwin32])

GLOBAL_CFLAGS=$ACCFLAGS
AC_SUBST(GLOBAL_CFLAGS)

LIBDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
GLOBAL_LIBS=" \
 $LIBDIR/libimobiledevice.a \
 $LIBDIR/libplist.a \
 $LIBXML2_STATIC \
 $OPENSSL_STATIC \
 $LIBDIR/libusbmuxd.a \
 $LIBDIR/libbz2.a \
 $ACSHAREDLIBS \ 
"
AC_SUBST(GLOBAL_LIBS)

GLOBAL_LDFLAGS="-static-libgcc $ACLDFLAGS"
AC_SUBST(GLOBAL_LDFLAGS)

GUI_CFLAGS=$ACGUI_CFLAGS
AC_SUBST(GUI_CFLAGS)

GUI_LDFLAGS=$ACGUI_LDFLAGS
AC_SUBST(GUI_LDFLAGS)

GUI_LDADD=$ACGUI_LDADD
AC_SUBST(GUI_LDADD)

AC_CONFIG_FILES(Makefile src/Makefile tools/Makefile gui/Makefile)
AC_OUTPUT

