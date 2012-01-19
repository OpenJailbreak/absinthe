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

mkdir -p build/absinthe/cli/data/common/corona
mkdir -p build/absinthe/gui/data/common/corona

case `uname` in
	Darwin)
	cp src/absinthe build/absinthe/cli/
	cp gui/absinthe build/absinthe/gui/
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

# common files
cp webclip_icon.png build/absinthe/cli/data/
cp webclip_Info.plist build/absinthe/cli/data/
cp webclip_icon.png build/absinthe/gui/data/
cp webclip_Info.plist build/absinthe/gui/data/

cp -r data/common/corona build/absinthe/cli/data/common/
cp -r data/common/corona build/absinthe/gui/data/common/

cp iOS/cleanup/cleanup build/absinthe/cli/data/common/corona/
cp iOS/filemover/filemover build/absinthe/cli/data/common/corona/
cp iOS/filemover/filemover.plist build/absinthe/cli/data/common/corona/
cp iOS/cleanup/cleanup build/absinthe/gui/data/common/corona/
cp iOS/filemover/filemover build/absinthe/gui/data/common/corona/
cp iOS/filemover/filemover.plist build/absinthe/gui/data/common/corona/


# device specific files

function devfiles()
{
  for I in data/$1/*; do
    mkdir -p build/absinthe/cli/$I
    mkdir -p build/absinthe/gui/$I
    cp -r $I/corona build/absinthe/cli/$I/
    cp -r $I/corona build/absinthe/gui/$I/
    cp -r $I/fsgen build/absinthe/gui/$I/
    cp -r $I/fsgen build/absinthe/gui/$I/
  done
}

devfiles 9A334
devfiles 9A405
devfiles 9A406

