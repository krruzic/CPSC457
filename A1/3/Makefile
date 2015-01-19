obj-m += crcdev.o

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install
	gcc test.c -o test.o
clean:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.o
