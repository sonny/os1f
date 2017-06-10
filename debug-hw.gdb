# GDB command file
# use: gdb -q -x debug.gdb ELF-file

set mem inaccessible-by-default off
set confirm off
#set target-async on

set listsize 100

target remote localhost:3333
monitor reset halt
#monitor arm semihosting enable

#layout src
layout asm
layout regs
focus cmd
load
tbreak main
continue

