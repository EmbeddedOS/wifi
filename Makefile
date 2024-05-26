obj-m += vwlan.o
all:
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc  wifi_client.c -I /usr/include/libnl3/ -lnl-genl-3 -lnl-3 -o wifi_client.o
