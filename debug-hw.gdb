# GDB command file
# use: gdb -q -x debug.gdb ELF-file

set mem inaccessible-by-default off
set confirm off
#set target-async on

set listsize 100
set output-radix 16

target remote localhost:3333
monitor reset halt

monitor arm semihosting enable
#monitor tpiu config internal itm.fifo uart off 8000000

tui reg all
#layout split
layout regs
layout src
focus cmd
load
tbreak main
continue

