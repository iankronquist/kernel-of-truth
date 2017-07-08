Kernel of Truth [![build_status](https://travis-ci.org/iankronquist/kernel-of-truth.svg?branch=master)](https://travis-ci.org/iankronquist/kernel-of-truth)
===============

A simple kernel written in C.

The goal of this project is to build a simple kernel which can serve as a
platform for future experimentation. The Kernel of Truth includes a variety of
modern security features and mitigations.
This project currently targets the x86_64 architecture. In the future we may
target ARMv7 or x86.

Kernel Features
---------------
- [x] Support for x86_64.
- [x] Boots from multiboot v1 compliant bootloader.
- [x] 🔐 Supervisor Memory Access & Execution Protection (SMEP & SMAP).
- [x] 🔐 Write XOR Execute memory permissions and Data Execution Prevention.
- [x] 🔐 Heap red zone protection.
- [x] 🔐 Stack guard protection.
- [x] Undefined behavior sanitation (debug builds only).
- [x] 8259 Programmable Interrupt Controller.
- [x] 🔐 Process memory isolation.
- [x] RS232 serial.
- [x] 8253 Programmable Interrupt Timer.
- [x] Preemptive multitasking.
- [x] Higher half.
- [x] Virtual File System.
- [x] PS2 keyboard driver.
- [x] VGA graphics driver.
- [x] CMOS Real Time Clock.
- [x] 🔐 Port of Libsodium ed25519 & SHA512 cryptographic algorithms.
- [x] Elf loader.
- [x] Loadable kernel modules.
- [x] 🔐 Kernel module signature verification.
- [x] 🔐 Kernel module Address Space Layout Randomization (ASLR).
- [ ] Basic syscalls.
- [ ] POSIX syscall layer.
- [ ] EXT2 filesystem.
- [ ] ATA.
- [ ] Symmetric Multiprocessing (SMP).


Bug Reports & Contributing
--------------------------
Advice, ideas, bug reports and pull requests are all welcome! Please take a
look at the [issue tracker][issues] on GitHub. Please submit PRs via GitHub,
although I will also take patches via email.

Getting Started
---------------
First build an x86_64 GCC and binutils. There is a handy script which works on
most Linuxes and OS X in `scripts/build_cross_compiler.sh`.
Then install qemu with your favorite package manager. We'll need
`qemu-system-x86_64` to run the kernel. Once you have all this installed
building and running this little kernel is quite easy:
```
# Build the kernel
$ make
# Start it with qemu
$ make start
```

The Makefile also has some targets to make debugging easier.
* `make debug`: Include undefined behavior sanitation and debug symbols.
* `make release`: Strip symbols.
* `make start-log`: Log all assembly instructions executed and interrupts fired
  to the file `qemu.log`. Careful, this file can grow to be tens of megabytes
  in a few seconds!

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
[issues]:https://github.com/iankronquist/kernel-of-truth/issues
