# /dev/one

A device that when read from, returns '1' in ascii, this is a learning project -
don't take this code as an example :)

## Building

Running make should work for your system, if not, try installing linux-headers
using your distributions package manager.

```
make
```

To load the module after building, do

```
sudo insmod src/dev_one.ko
```

After this, you can read from /dev/one !

```
sudo head -c 20 /dev/one
```
