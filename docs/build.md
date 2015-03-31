Simple builds
=============

Everything:

```
make
```

Only `v7` cmdline binary:

```
make v7
```

Run unit tests and ECMA test suite:

```
make run
```


Advanced builds
===============

Variants
--------

There are several build flavours, `test_xxx`, where xx is one of:

- C dialects:
 - ansi
 - c99
 - c11
 - c++
- instrumentation:
 - asan
 - msan
 - gcov

Example:

```
make -C tests test_asan
```

Flags
-----

You can pass extra flags with `CFLAGS_EXTRA`:

```
make v7 CFLAGS_EXTRA="-DV7_DISABLE_REGEX -DV7_DISABLE_SOCKET"
```

Misc
----

`make setup-hooks` will install hooks in your local git repository which
ensure that:

- you don't forget to amalgamate before committing
- your sources are formatted according to our guidelines
- sanity checks on the commit comment style

Dependencies
------------

Advanced builds require more tools, which you might not have on your local machine,
you might not have the right version, have features disabled by default (ASan on OSX)
or are not even available (e.g. MSan on OSX).

In order to help make sure that anybody can build everything, and do it with exactly the same
environment as others (and the CI) we bundle a full development environment as a docker image `cesanta/v7_test`.
You can easily run docker on OSX and windows using http://boot2docker.io

The current working directory gets mounted inside the docker container. This works seamlessly
through virtualbox shared folders using boot2docker as well.

You can invoke builds in a pristine docker container using the `make.sh` wrapper. It accepts the same
arguments as a local build.

Cross compilation
-----------------

You can build windows executables with MSVC6:

```
./make.sh w
```

(You can also setup MSVC6 and wine locally if you prefer, but I leave that as an excercise for the reader)


Currently the build environment has these pre-installed build targets:

- x86_64:
    - linux: gcc-4.9.2, clang-3.6.0
    - win64: mingw-gcc-4.9.2
- x386:
    - linux: gcc-4.9.2, clang-3.6.0
    - win32: MSVC6, openwatcom, mingw-gcc-4.9.2
- 086:
    - dos: openwatcom-1.9

Currently the makefiles support only MSVC6 build (`make w`),
but other targets should be added easily.

Build image
-----------

The build image is defined in `tests/Dockerimage`.
Add new toolchains by adding `RUN` stanzas, possibly after the existing entries that run `apt-get`
in order to leverage docker image caching.

In order to build the build image you need two dependencies that we don't bundle in our git repo:

- `tests/openwatcom-linux-cross-1.9.tar.xz`
- `tests/vc6.tar.xz`

Image is built with:

```
docker build -t cesanta/v7_test
```

And pushed to docker registry with:

```
docker push cesanta/v7_test
```
