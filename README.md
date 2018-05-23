# simple_os
Simple OS for ARM v7 embedded processors

## Legal Stuff
All of the files/code in this project are open-source in one way or another.
__HOWEVER__, I have not added copyright headers to any of the code. Some files
already have copyright headers from their sources. In other words, the legal
status of the overall project is ambiguous. Please feel free to encourage me
to sort them out if you end up wanting to use the project.

## Purpose
I created this project in order to learn about low level OS stuff. This means
that most of the implementation is relatively simple. In some cases, I make
the implementation more complex than a minimal setup would imply. Basically,
I am adding whatever I feel like, whenever I feel like it.

This OS can be used to implement more complex embedded projects on ARM v7 
embedded processors (such as the M7 and M4). Specifically, my development is 
targeted at the [32F746GDISCOVERY development board](http://www.st.com/en/evaluation-tools/32f746gdiscovery.html).

It is still very much a work in progress and the API changes constantly.

## Features
* Fully Preemtive Multitasking OS
* Integrates with STI's embeded library to simplify use of on-chip peripherals and busses
* Shell on the Virtual COM Port allows some diagnostic interaction, including task state and stack backtracing.
* Concurrency primitives - Spinlocks, Mutexes
* Events and Timers
* Watchdog timer
* Heap allocator (malloc and free)
* Multiple schedulers available (trivial, round-robin, and priority) can be selected at compile time.
* Low power mode in the idle task.
* Serial IO (can also output text to the LCD)
* VT100 emulation for creating formatted Serial output
* Virtual LEDs -- emulate multiple blinking bits on the LCD, since the board only has one real LED.



