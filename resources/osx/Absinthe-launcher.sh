#!/bin/sh
C=`dirname "${0}"`

case `sysctl -n kern.osversion |sed 's/[A-Z]/:/g' |cut -d ":" -f 1` in
11|10)
	exec "$C/Absinthe-10.6"
	;;
9)
	exec "$C/Absinthe-10.5"
	;;
*)
	exit 1
	;;
esac
exit 0
