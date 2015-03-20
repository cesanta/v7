FROM ubuntu:vivid

MAINTAINER Marko Mikulicic <mkm@cesanta.com>

RUN apt-get update && apt-get install -y --no-install-recommends build-essential libssl-dev
RUN apt-get install -y --no-install-recommends clang llvm clang-format-3.6 libunwind8-dev
RUN apt-get install -y --no-install-recommends g++-multilib libc6-dev-i386 git
RUN ln -s /usr/bin/llvm-symbolizer-3.6 /usr/bin/llvm-symbolizer
RUN ln -s /usr/bin/clang-format-3.6 /usr/bin/clang-format

VOLUME ['/cesanta']

ENTRYPOINT make -C /cesanta/v7 run CFLAGS_EXTRA=-DTEST_UNDER_VIRTUALBOX
