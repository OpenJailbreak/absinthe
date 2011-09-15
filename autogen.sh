#!/bin/sh
aclocal
autoconf
autoheader
automake -a -c
autoreconf -i
CFLAGS="-g -arch i386" ./configure
