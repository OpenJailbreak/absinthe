#!/bin/bash

if [ "x$1" = "xclean" ]; then
  rm -f build
  make clean
fi

if ! test -f configure; then
    ./autogen.sh
fi

rm -rf build
if ! make; then
  exit 1
fi

CLIDEST=build/absinthe/cli
GUIDEST=build/absinthe/gui

mkdir -p $CLIDEST
mkdir -p $GUIDEST

case `uname` in
	Darwin)
	OSX_BUNDLE_NAME=Absinthe
	cp src/absinthe build/absinthe/cli/
	mkdir -p $GUIDEST/${OSX_BUNDLE_NAME}.app/Contents/MacOS
	mkdir -p $GUIDEST/${OSX_BUNDLE_NAME}.app/Contents/Resources
	echo "APPL????" > $GUIDEST/${OSX_BUNDLE_NAME}.app/Contents/PkgInfo
	cp resources/osx/Info.plist $GUIDEST/${OSX_BUNDLE_NAME}.app/Contents/
	cp resources/osx/Icon.icns $GUIDEST/${OSX_BUNDLE_NAME}.app/Contents/Resources/
	GUIDEST=$GUIDEST/${OSX_BUNDLE_NAME}.app/Contents/MacOS
	cp gui/absinthe $GUIDEST/${OSX_BUNDLE_NAME}
	# if running as root required:
	#cp gui/absinthe $GUIDEST/${OSX_BUNDLE_NAME}_
	#cp resources/osx/launcher $GUIDEST/${OSX_BUNDLE_NAME}
	;;
	Linux)
	cp src/absinthe build/absinthe/cli/
	cp gui/absinthe build/absinthe/gui/
	;;
	MINGW*)
	cp src/absinthe.exe build/absinthe/cli/
	cp gui/absinthe.exe build/absinthe/gui/
	;;
esac

mkdir -p $CLIDEST/data/common/corona
mkdir -p $GUIDEST/data/common/corona

# common files
cp data/common/webclip_icon*.png $CLIDEST/data/common/
cp data/common/webclip_Info.plist $CLIDEST/data/common/
cp data/common/webclip_icon*.png $GUIDEST/data/common/
cp data/common/webclip_Info.plist $GUIDEST/data/common/

cp -r data/common/corona $CLIDEST/data/common/
cp -r data/common/corona $GUIDEST/data/common/

cp iOS/cleanup/cleanup $CLIDEST/data/common/corona/
cp iOS/filemover/filemover $CLIDEST/data/common/corona/
cp iOS/filemover/filemover.plist $CLIDEST/data/common/corona/
cp iOS/cleanup/cleanup $GUIDEST/data/common/corona/
cp iOS/filemover/filemover $GUIDEST/data/common/corona/
cp iOS/filemover/filemover.plist $GUIDEST/data/common/corona/


# device specific files

function devfiles()
{
  for I in data/$1/*; do
    mkdir -p $CLIDEST/$I
    mkdir -p $GUIDEST/$I
    cp -r $I/corona $CLIDEST/$I/
    cp -r $I/corona $GUIDEST/$I/
    cp -r $I/fsgen $CLIDEST/$I/
    cp -r $I/fsgen $GUIDEST/$I/
  done
}

devfiles 9A334
devfiles 9A405
devfiles 9A406

