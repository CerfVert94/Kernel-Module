obj-m:=kpwm.o

KDIR := /home/sasl/shared/saintbauzel/BBB/linux/build

all: default execs


default:
	$(MAKE) CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm  -C ${KDIR} SUBDIRS=$(shell pwd) modules

clean:
	rm -f *.[oas] .*.flags *.ko .*.cmd .*.d .*.tmp *.mod.c ioctl_pwm ioctl_gpio

execs: 
	/home/sasl/shared/saintbauzel/BBB/arm-linux-gnueabihf/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -o ioctl_gpio ioctl_gpio.c
	/home/sasl/shared/saintbauzel/BBB/arm-linux-gnueabihf/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -o ioctl_pwm ioctl_pwm.c

