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

ACCFLAGS="-m32 -I\$(top_srcdir)/include -DLIBXML_STATIC=1 -g -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES "
ACSHAREDLIBS=""

SYSTYPE=
case "$host_os" in
	darwin*)
		SYSTYPE=osx
		ACCFLAGS+="-fPIC -arch i386 "
		ACSHAREDLIBS+="-lpthread -lz -lm -liconv "
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		LIBWXDIR="\$(top_srcdir)/../wxWidgets-2.9.2"
		ACGUI_CFLAGS+="-DPIC -DHAVE_ASPRINTF -DHAVE_VASPRINTF \
			-isysroot /Developer/SDKs/MacOSX10.6.sdk \
			-DCURL_PULL_SYS_SOCKET_H \
			-D__WXOSX_COCOA__ -DwxDEBUG_LEVEL=0 \
			-I$LIBWXDIR/include -I$LIBWXDIR/lib/wx/include/osx_cocoa-unicode-static-2.9"
		ACGUI_LDFLAGS+="pthread"
		ACGUI_LIBS+="$LIBSDIR/libwxpng-2.9.a \
			$LIBSDIR/libwx_baseu-2.9.a \
			$LIBSDIR/libwx_osx_cocoau_core-2.9.a \
			-framework IOKit -framework Carbon -framework Cocoa -framework QuickTime -framework OpenGL -framework System -framework Security"
	;;
	linux*)
		SYSTYPE=linux
		ACCFLAGS+="-fPIC "
		ACSHAREDLIBS+="-lpthread -lz -lm "
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		LIBWXDIR="\$(top_srcdir)/../wxWidgets-2.9.2"
		ACGUI_CFLAGS+="-DPIC -DHAVE_ASPRINTF -DHAVE_VASPRINTF \
			-DwxDEBUG_LEVEL=0 \
			-I$LIBWXDIR/include \
			-I$LIBWXDIR/lib/wx/include/i686-linux-gnu-gtk2-unicode-static-2.9 \
			-D__WXGTK__ -pthread"
		ACGUI_LDFLAGS+="-pthread -Wl,-Bsymbolic-functions -L/lib32 -L/usr/lib32"
		ACGUI_LDADD+="$LIBSDIR/libwx_gtk2u_core-2.9.a \
			$LIBSDIR/libwx_baseu-2.9.a \
			-lpthread -lz -lm -lglib-2.0 -lgtk-x11-2.0"
	;;
	mingw32)
		SYSTYPE=win32
		ACCFLAGS+="-DWIN32 "
		ACSHAREDLIBS+="-lz -lm -lws2_32 -lgdi32 -lole32 "
		LIBSDIR="\$(top_srcdir)/staticlibs/$SYSTYPE"
		LIBWXDIR="\$(top_srcdir)/../wxWidgets-2.8.12"
		ACGUI_CFLAGS+="-D_WIN32 -D__LITTLE_ENDIAN__=1 \
			-DCURL_PULL_WS2TCPIP_H=1 \
			-I$LIBWXDIR/include \
			-I$LIBWXDIR/lib/wx/include/msw-unicode-release-static-2.8 \
			-D__WXMSW__ -DwxDEBUG_LEVEL=0 -mthreads"
		ACGUI_LDFLAGS+="-mthreads -Wl,--subsystem,windows -mwindows"
		ACGUI_LDADD+="$LIBSDIR/libwx_mswu_core-2.8.a \
			$LIBSDIR/libwx_baseu-2.8.a \
			-lz -lm -lws2_32 -lgdi32 -lole32 -loleaut32 -luuid -lcomctl32 -lsetupapi"
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

GUI_CFLAGS=$ACGUI_CFLAGS
AC_SUBST(GUI_CFLAGS)

GUI_LDFLAGS=$ACGUI_LDFLAGS
AC_SUBST(GUI_LDFLAGS)

GUI_LDADD=$ACGUI_LDADD
AC_SUBST(GUI_LDADD)

AC_CONFIG_FILES(Makefile src/Makefile tools/Makefile gui/Makefile)
AC_OUTPUT

