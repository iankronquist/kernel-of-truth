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
	* There are a few other libraries you can download if you'd like, which can be found in the externally linked instructions above.  They aren't necessary for building the kernel though.

b. Find a place to install your cross-compiler.  Generally, putting them in "../cross/bin" relative to the project root should be sufficient, but really you can put them wherever you'd like so long as you know how to access them.  Just be sure that your directory does not interfere with any system directories.

c. Once you've got your directory set up, run:
.. code:: bash
	
	export PREFIX="$HOME/opt/cross"
	export TARGET=i686-elf
	export PATH="$PREFIX/bin:$PATH"

Where PREFIX is the path to the directory you've created.  

d. Next compile your binutils:
.. code:: bash
	cd $HOME/src
	mkdir build-binutils
	cd build-binutils
	../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror
	make
	make install

e. Now we're officially ready to build a cross-compiler!
.. code:: bash
	
	cd $HOME/src
	 
	 # If you wish to build these packages as part of gcc:
	 mv libiconv-x.y.z gcc-x.y.z/libiconv # Mac OS X users
	 mv gmp-x.y.z gcc-x.y.z/gmp
	 mv mpfr-x.y.z gcc-x.y.z/mpfr
	 mv mpc-x.y.z gcc-x.y.z/mpc
	  
	 mkdir build-gcc
	 cd build-gcc
	 ../gcc-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
	 make all-gcc
	 make all-target-libgcc
	 make install-gcc
	 make install-target-libgcc

Sweet!  Now we have a very basic compiler.  Please keep in mind that you can't access any C libraries or create runnable binaries with this, but it will be sufficient for building the kernel.

2. To actually compile the kernel, just run:  
.. code-block:: bash
	
	$ make

3. To start the kernel, run:
.. code-block:: bash
	
	$ make start



