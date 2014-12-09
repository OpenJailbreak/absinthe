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
		cp resources/osx/Icon.icns ${SRCDIR}/.VolumeIcon.icns
		SetFile -c icnC "${SRCDIR}/.VolumeIcon.icns"
		hdiutil create -srcfolder "${SRCDIR}" -volname "Absinthe ${VER}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 128M temp.dmg
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
		cd build/absinthe
		7z a -sfx..\\..\\resources\\win32\\7zCon.sfx -t7z -m0=lzma -mx=9 -r $PD/$PKGNAME.exe $PKGNAME
		cd ${PD}
		rm -f $PKGNAME.zip
		mkdir winpkg
		cp $PKGNAME.exe winpkg/
		echo "Greenpois0n Absinthe $VER Windows Self-Extractor
-------------------------------------------------

Please extract the absinthe-win-$VER.exe to a reasonable location like
the Desktop or your My Documents folder, then double click it.
This will extract the whole Absinthe files into a new folder called
'absinthe-win-$VER'.
Open this folder and run absinthe.exe to run the actual Absinthe
jailbreak application.

Good luck!
" >> winpkg/readme.txt
		conv --u2d winpkg/readme.txt
		cd winpkg
		7z a -tzip -mx=9 -r ${PD}/$PKGNAME.zip $PKGNAME.exe readme.txt
		cd $PD
		rm -rf winpkg
	;;
esac
