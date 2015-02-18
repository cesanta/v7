FROM ubuntu:trusty

MAINTAINER Marko Mikulicic <mkm@cesanta.com>

RUN apt-get update && apt-get install -y --no-install-recommends build-essential libssl-dev
RUN apt-get install -y --no-install-recommends clang-3.5 llvm-3.5 libunwind8-dev
RUN ln -s /usr/bin/llvm-symbolizer-3.5 /usr/bin/llvm-symbolizer

VOLUME ['/cesanta']

ENTRYPOINT make -C /cesanta/v7 run CFLAGS_EXTRA=-DTEST_UNDER_VIRTUALBOX
