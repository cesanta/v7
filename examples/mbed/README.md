Online IDE
==========

Open https://developer.mbed.org/compiler,
import the `v7` library and upload the `main.cpp`
file you find in this directory.

Command-line build
=================

If you prefer to build the example from commandline
you can use the `mbed` command line tool. Find out
more about it, including installation instructions
at https://github.com/ARMmbed/mbed-cli .

This example directory contains only a small source file
and two library "pointers", one for the core mbed-os, and one
for the V7 library itself. In order to download the dependencies
you need to invoke:

    mbed update default
    mbed deploy

Then you need to pick a toolchain and a target platform, e.g.

    mbed toolchain GCC_ARM
    mbed target DISCO_F469NI

Now you can compile this example:

    mbed compile
