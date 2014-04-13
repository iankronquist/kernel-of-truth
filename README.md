Kernel of Truth
===============

A simple kernel written in C.

The goal of this project is to build a simple kernel with memory protection and 
iterrupts which can run simple programs.
This project currently targets the i586 architecture. Perhaps someday I will 
port it to ARM so it can run on the Raspberry Pi, but right now I'm focused on 
implementing the fundamentals of a system.

Pull requests are welcome.

Tutorials & Resources
---------------------
The following tutorials have proven useful in developing this kernel:
* The OSDev wiki [Bare Bones Tutorial][0]
* The OSDev wiki [Meaty Skeleton Tutorial][1]
* The [OSDever tutorial][2]

The following resources may also prove useful:
* Info on the Global Descriptor Table:
	http://wiki.osdev.org/GDT_Tutorial
	http://www.osdever.net/bkerndev/Docs/gdt.htm
* Info on the Interrupt Descriptor Table:
	http://www.osdever.net/bkerndev/Docs/idt.htm
* Info on the Interrupt Service Routines:
	http://www.osdever.net/bkerndev/Docs/isrs.htm
* Info on implementing `malloc` and `free`:
	http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf
	https://codereview.stackexchange.com/questions/19635/malloc-free-implementation
	http://wiki.osdev.org/Memory_Allocation
* Info on PS/2 Keyboard drivers:
	http://wiki.osdev.org/PS/2_Keyboard
* Info on OS Development on the Pi:
	https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/index.html
	http://elinux.org/RPi_Easy_SD_Card_Setup

Building:
---------
Follow the instructions on the OSDev wiki [Bare Bones Tutorial][0]. You
will need to build a GCC crosscompiler and binutils for the i586 architecure.
Put the linker, assembler, and gcc binaries in `../bin/bin/`, relative to 
the Makefile. Alternately, change lines 2 and 3 of the Makefile to point
to your binaries.

To Do:
------

- [ ] Get stdio working. I can print to the terminal using `term_putchar()`,
but I have had issues with declaring `printf`, `puts`, and other functions. 
Look at the following files:  
	* kernel/terminal.h  
	* tlibc/stdio/stdio.c  
	* tlibc/stdio/stdio.h   
- [ ] Implement the following interrupt tables:  
	- [ ] GDT  
	- [ ] IDT  
	- [ ] ISRs  
- [ ] Write a memory manager to use `malloc` and `free`. 
- [ ]  Write a keyboard driver, so I can type on the screen.
- [ ]  Write tests. There aren't many right now, they're located in the directory 
`tests`
- [ ]  Write more comments and documentation. Again, it's kind of spartan right now.
- [ ]  Research how to get a kernel to boot on the Pi. Half the kernel will need
to be rewritten to target ARM, but that will help form and organize the project.
A cross compiler for the Pi written by the Raspberry Pi foundation can be found
on [github][3]  


[0]:http://wiki.osdev.org/C%2B%2B_Bare_Bones
[1]:http://wiki.osdev.org/User:Sortie/Meaty_Skeleton
[2]:http://www.osdever.net/bkerndev/Docs/gettingstarted.htm
[3]:https://github.com/raspberrypi/tools
