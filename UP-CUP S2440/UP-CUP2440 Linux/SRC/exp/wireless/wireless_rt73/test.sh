insmod rt73.ko
ifconfig eth0 down
ifconfig rausb0 192.168.1.199 up
route add default gw 192.168.1.254 dev rausb0
ifconfig -a
iwlist scanning

