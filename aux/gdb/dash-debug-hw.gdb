# GDB command file
# use: gdb -q -x debug.gdb ELF-file

set mem inaccessible-by-default off
set confirm off
#set target-async on

#set listsize 100

source dashboard.gdb
dashboard source -style context 10
dashboard assembly -style context 5


target remote localhost:3333
monitor reset halt
monitor arm semihosting enable




load
tbreak main
continue
