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
		SRCDIR="build/absinthe/${PKGNAME}"
		MNT="/tmp/dmgmnt"
		rm -rf ${MNT}
		rm -f $PKGNAME.dmg
		rm -f temp.dmg
		hdiutil create -srcfolder "${SRCDIR}" -volname "Absinthe ${VER}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 120M temp.dmg
		cp resources/osx/Icon.icns ${SRCDIR}/.VolumeIcon.icns
		SetFile -c icnC "${SRCDIR}/.VolumeIcon.icns"
		mkdir -p ${MNT}
		hdiutil attach temp.dmg -mountpoint ${MNT}
		SetFile -a C ${MNT}
		hdiutil detach ${MNT}
		rm -rf ${MNT}
		hdiutil convert temp.dmg -format UDBZ -o $PKGNAME.dmg
		rm -f temp.dmg
	;;
	Linux*)
		rm $PKGNAME.tar.bz2
		tar cjvf $PKGNAME.tar.bz2  -C build/absinthe $PKGNAME
	;;
	MINGW*)
		rm -f $PKGNAME.exe
		PD=`pwd`
		7z a -sfxresources\\win32\\7zCon.sfx -t7z -m0=lzma -mx=9 -r $PD/$PKGNAME.exe -w build/absinthe $PKGNAME
	;;
esac
