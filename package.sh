#!/bin/bash

VER=`grep ABSINTHE_VERSION_STRING src/version.h |cut -d " " -f 3 |sed 's/"//g'`
PKGNAME=absinthe-$VER

case `uname` in
	Darwin*)
		PKGNAME=absinthe-mac-$VER
	;;
	Linux*)
		PKGNAME=absinthe-linux-$VER
	;;
	MINGW*)
		PKGNAME=absinthe-win-$VER
	;;
esac

if ! test -d build/absinthe/$PKGNAME; then
	./build.sh
fi

case `uname` in
	Darwin*)
		rm $PKGNAME.zip
		PD=`pwd`
		cd build/absinthe
		zip -9 -r $PD/$PKGNAME.zip $PKGNAME
		cd $PD
	;;
	Linux*)
		rm $PKGNAME.tar.gz
		tar czvf $PKGNAME.tar.gz  -C build/absinthe $PKGNAME
	;;
	MINGW*)
		rm $PKGNAME.zip
		PD=`pwd`
		cd build/absinthe
		7z a -mx9 -r $PD/$PKGNAME.zip $PKGNAME
		cd $PD
	;;
esac

