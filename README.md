V7: Embedded JavaScript engine
==============================

**NOTE: this project is deprecated in favor of https://github.com/cesanta/mjs**

[![License](https://img.shields.io/badge/license-GPL_2-green.svg)](https://github.com/cesanta/v7/blob/master/LICENSE)

V7 is the smallest JavaScript engine written in C. V7 features are:

- Cross-platform: works on anything, starting from Arduino to MS Windows
- Small size. Compiled static size is in 40k - 120k range, RAM
  footprint on initialization is about 800 bytes with freeze feature,
  15k without freeze feature
- Simple and intuitive C/C++ API. It is easy to export existing C/C++
  functions into JavaScript environment
- Standard: V7 implements JavaScript 5.1
- Usable out-of-the-box: V7 provides an auxiliary library with
  Hardware (SPI, UART, etc), File, Crypto, Network API
- Source code is both ISO C and ISO C++ compliant
- Very easy to integrate: simply copy two files: [v7.h](v7.h)
   and [v7.c](v7.c) into your project

V7 makes it possible to program Internet of Things (IoT) embedded devices
in JavaScript. V7 is a part of the full stack
[Mongoose OS Platform](https://github.com/cesanta/mongoose-os).

## Examples & Documentation

- [Developer Centre](https://docs.cesanta.com/v7/dev) - User Guide and API reference
- [Examples](https://github.com/cesanta/v7/tree/master/examples) - Collection of well-commented examples
- [Support Forum](http://forum.cesanta.com/index.php?p=/categories/v7) - Ask questions on our support forum

# Contributions

To submit contributions, sign
[Cesanta CLA](https://docs.cesanta.com/contributors_la.shtml)
and send GitHub pull request. You retain the copyright on your contributions.

# Licensing

V7 is released under commercial and [GNU GPL v.2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.html) open source licenses.

Commercial Projects:
Once your project becomes commercialised GPLv2 licensing dictates that you need to either open your source fully or purchase a commercial license. Cesanta offer full, royalty-free commercial licenses without any GPL restrictions. If your needs require a custom license, we’d be happy to work on a solution with you. [Contact us for pricing.] (https://www.cesanta.com/contact)

Prototyping:
While your project is still in prototyping stage and not for sale, you can use V7’s open source code without license restrictions.
