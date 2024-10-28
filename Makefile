default: modules

modules:
	make -C /lib/modules/$(shell uname -r)/build \
		M=$(PWD) \
		modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: default clean modules
