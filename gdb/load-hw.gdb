# GDB command file
# use: gdb -q -x load-hw.gdb ELF-file

set mem inaccessible-by-default off
set confirm off
set target-async on

target remote localhost:3333
monitor reset halt

# load flash
load

# reset and run MCU from openocd
monitor reset run

# exit openocd server (disconnects from gdb)
monitor shutdown

# exit gdb
quit


