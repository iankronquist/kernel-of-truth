Kernel of Truth
===============

A simple kernel written in C.

The goal of this project is to build a simple kernel with memory protection and 
iterrupts which can run simple programs.
This project currently targets the x86 architecture. Perhaps someday I will 
port it to ARM so it can run on the Raspberry Pi, but right now I'm focused on 
implementing the fundamentals of a system.

Tutorials & Resources
---------------------
The following tutorials have proven useful in developing this kernel:
* The OSDev wiki [Bare Bones Tutorial][0]
* The OSDev wiki [Meaty Skeleton Tutorial][1]
* The [OSDever tutorial][2]

The following resources may also prove useful:
* Info on implementing `malloc` and `free`:
	http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf
	https://codereview.stackexchange.com/questions/19635/malloc-free-implementation
	http://wiki.osdev.org/Memory_Allocation
* Info on OS Development on the Pi:
	https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/index.html
	http://elinux.org/RPi_Easy_SD_Card_Setup

After a certain point the tutorials become significantly less useful. It is
helpful to refer to implementations of several hobbyist and professional
kernels including:

* [ToaruOS](https://github.com/klange/toaruos)
* [Minix 3](https://github.com/minix3/minix)
* [xv6](https://pdos.csail.mit.edu/6.828/2014/xv6.html)
* and, if you are desperate, [Linux](https://github.com/torvalds/linux/)

If you prefer reading textbooks, Tanenbaum's *Operating Systems Design and
Implementation* is quite useful. Copies can be found floating around the
internet. The [FreeBSD developer's handbook][4] can also offer useful insights.
The [Little OS Book][5] is approachable, but hardly thorough.

Getting Started
---------------
First build an x86 GCC and binutils. If you're not familiar with how to do this
you should consult the [Bare Bones Tutorial][0].
Then install qemu with your favorite package manager. We'll need
`qemu-system-i386` to run the kernel. Once you have all this installed building
and running this little kernel is quite easy:
```
# Build the kernel
$ make
# Start it with qemu
$ make start
```

The Makefile also has some targets to make debugging easier.
* `make start-log`: log all assembly instructions executed and interrupts fired
  to the file `qemu.log`. Careful, this file can grow to be tens of megabytes
  in a few seconds!
* `make start-debug`: Start qemu in debug mode so you can connect to it with
  GDB. To connect with GDB run:

```
$ gdb
(gdb) target remote localhost:1234
(gdb) continue
```


[0]:http://wiki.osdev.org/C%2B%2B_Bare_Bones
[1]:http://wiki.osdev.org/User:Sortie/Meaty_Skeleton
[2]:http://www.osdever.net/bkerndev/Docs/gettingstarted.htm
[3]:https://github.com/raspberrypi/tools
[4]:https://www.freebsd.org/doc/en/books/developers-handbook/
[5]:http://littleosbook.github.io/book.pdf
