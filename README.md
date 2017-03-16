Kernel of Truth [![build_status](https://travis-ci.org/iankronquist/kernel-of-truth.svg?branch=master)](https://travis-ci.org/iankronquist/kernel-of-truth)
===============

A simple kernel written in C.

The goal of this project is to build a simple kernel which can serve as a
platform for future experimentation.
This project currently targets the x86_64 architecture. In the future we may
target ARMv7 or x86.

Tutorials & Resources
---------------------
In OS development, nothing compares to the manufacturer's manuals, especially
the [Intel x86 manuals][0]. However, a variety of other resources have proven
useful when first getting started.
* The OSDev wiki [Bare Bones Tutorial][1]
* The OSDev wiki [Meaty Skeleton Tutorial][2]
* The [OSDever tutorial][3]


It can be helpful to refer to implementations of other hobbyist and
professional kernels such as:

* [ToaruOS](https://github.com/klange/toaruos)
* [Minix 3](https://github.com/minix3/minix)
* [Linux](https://github.com/torvalds/linux/)
* [freebsd](https://github.com/freebsd/freebsd)

If you prefer reading textbooks, Tanenbaum's *Operating Systems Design and
Implementation* is quite useful. Copies can be found floating around the
internet. The [FreeBSD developer's handbook][5] can also offer interesting
insights from more of a userland perspective.

Bug Reports & Contributing
--------------------------
Advice, ideas, bug reports and pull requests are all welcome! Please take a
look at the [issue tracker][issues] on GitHub. Please submit PRs via GitHub,
although I will also take patches via email.

Getting Started
---------------
First build an x86_64 GCC and binutils. If you're not familiar with how to do this
you should consult the [Bare Bones Tutorial][1].
Then install qemu with your favorite package manager. We'll need
`qemu-system-x86_64` to run the kernel. Once you have all this installed building
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

Code Style
----------

When in Rome, do as the Romans.

Most of the style is enforced via the uncrustify linter. Run it on your code
and call it mostly good. However, it doesn't enforce certain variable naming
conventions.

Use typedefs extremely sparingly. Prefer native C types instead. Always use the
most descriptive type available.
Do not use `ifdef` style header guards, use `#pragma once` instead.
Alphabetize `include` statements. They should be in paragraphs based on their
top level directory. Local `includes` should go last.

Leave a comment describing every symbol visible in a public header.
Otherwise, comment tactically and sparingly.

```
#include <external/a.h>
#include <external/b.h>
#include <external/c.h>

#include <truth/a.h>
#include <truth/b.h>
#include <truth/c.h>

#include "a.h"
#include "b.h"
#include "c.h"


struct types_like_this;

// Unless the type is an acronym.
struct GDT;

#define Global_Defines_Like_This 42
struct GDT Globals_Like_This;

void *functions_like_this(void *foo) {
    size_t locals_like_this = 15;
    return foo + locals_like_this;
}
```

[0]:http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html
[1]:http://wiki.osdev.org/C%2B%2B_Bare_Bones
[2]:http://wiki.osdev.org/User:Sortie/Meaty_Skeleton
[3]:http://www.osdever.net/bkerndev/Docs/gettingstarted.htm
[5]:https://www.freebsd.org/doc/en/books/developers-handbook/
[6]:http://littleosbook.github.io/book.pdf
[issues]:https://github.com/iankronquist/kernel-of-truth/issues
