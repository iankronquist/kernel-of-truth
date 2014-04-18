=========
Compiling
=========

Compiling my kernel is fairly simple, provided you use a cross compiler and are running some flavor of Linux.
You will need to install the following packages:
	* `qemu`_
	* `GCC`_ (for your local machine)
	* `G++`_ (if you're using a newer version of GCC)

To compile:
===========
1. Build a GCC cross compiler. This may sound daunting, but fear not!  It's actually a pretty easy process.  The following instructions can also be found `here <http://wiki.osdev.org/GCC_Cross-Compiler#Preparing_for_the_build>`_.

	a. Download the following libraries:
		* `GNU GMP <https://gmplib.org/>`_ --See the download link in the top left.
		* `GNU MPFR <http://www.mpfr.org/mpfr-current/#download>`_
		* `GNU MPC <http://multiprecision.org/index.php?prog=mpc&page=download>`_

	b. 
rename them.
move them to proper build directories.
Configure them. make and install.
Move the executables. I recommend putting them in ../cross/bin relative to the
project root. This way you will not have to edit the path to the cross compiler and linker in the Makefile.

To actually compile the kernel, just run:  
.. code-block:: bash
	$ make

To start the kernel, run:
.. code-block:: bash
	$ make start



