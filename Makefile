all: modules

modules:
	make -C /lib/modules/$(shell uname -r)/build \
		M=$(PWD) \
		EXTRA_CFLAGS="-O2" \
		modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: all clean modules
