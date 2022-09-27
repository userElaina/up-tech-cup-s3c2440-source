#!/bin/sh
init()
{
	/etc/init.d/bluetooth start
	pand --listen --role NAP
}
case "$1" in
  net)
  	ifconfig bnep0 10.0.0.1
  	;;
  *)
  	echo "init bluetooth"
  	init
  	exit 1
esac
exit 0

