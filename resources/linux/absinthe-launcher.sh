#!/bin/sh

DIR=`dirname $0`
PLAT=`uname -p`
case $PLAT in
	x86_64)
	${DIR}/absinthe.x86_64
	;;
	i586|i686)
	${DIR}/absinthe.x86
	;;
	*)
	echo Sorry, platform $PLAT is not supported.
	;;
esac
