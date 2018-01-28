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
that most of the implementation is relatively simple. For example, the task list
is just an array of task items and the scheduler simply iterates over the array
to find the next task to execute. This is somewhat sub-optimal, but serves a 
more important purpose of being easy to understand.

This OS can be used to implement more complex embedded projects on ARM v7 
embedded processors (such as the M7 and M4). Specifically, my development is 
targeted at the [32F746GDISCOVERY development board](http://www.st.com/en/evaluation-tools/32f746gdiscovery.html).

## Features
* Fully Preemtive Multitasking OS
* Very simple heap allocator
* Concurrency primitives - Mutexes (only atm)
* Events
* Serial IO (can also output text to the LCD)
* VT100 emulation for creating formatted Serial output
* Integrates with STI's embeded library to simplify use of on-chip peripherals and busses

