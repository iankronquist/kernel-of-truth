Compiling
=========

Compiling my kernel is simple - if you use a cross compiler.
You will need to install the following packages:
qemu
gcc (for your local machine)
First you need to build binutils a GCC cross compiler. This is not as daunting
as it may sound - it took maybe fourty-five minutes to compile and set up on a
core2duo laptop.
Instructions for this can be found on the OsDev wiki. 
I shall describe the process here.

Download the following libraries:
a
b
c
d

untar them.
rename them.
move them to proper build directories.
Configure them. make and install.
Move the executables. I recommend putting them in `../cross/bin` relative to the
project root. This way you will not have to edit the path to the cross compiler and linker in the Makefile.

To actually compile the kernel, just run:  
`$ make`  
To start the kernel, run:
`$ make start`
