#!/bin/sh
init()
{
        echo 115200 > /proc/sys/net/irda/max_baud_rate
	#irattach irda0 -s 1
	./irattach /dev/ttySAC2 -s 1
}

case "$1" in
  send)
        ./irxfer $2
        ;;
  resv)
        ./irxfer
        ;;
  *)
        echo "init irda"
        init
        #irdadump
        exit 1
esac
exit 0
