FROM ubuntu:vivid

MAINTAINER Marko Mikulicic <mkm@cesanta.com>

RUN apt-get update && apt-get install -y --no-install-recommends build-essential libssl-dev
RUN apt-get install -y --no-install-recommends clang llvm clang-format-3.6 libunwind8-dev
RUN apt-get install -y --no-install-recommends g++-multilib libc6-dev-i386 git
RUN ln -s /usr/bin/llvm-symbolizer-3.6 /usr/bin/llvm-symbolizer
RUN ln -s /usr/bin/clang-format-3.6 /usr/bin/clang-format

ADD openwatcom-linux-cross-1.9.tar.xz /opt
RUN echo 'export WATCOM=/opt/watcom' >>/root/.bashrc ; echo 'source $WATCOM/owsetenv.sh' >>/root/.bashrc; sed 's:/tmp:/opt:g' -i '/opt/watcom/owsetenv.sh'

RUN dpkg --add-architecture i386; apt-get update; apt-get install -y --no-install-recommends wine
RUN apt-get install -y --no-install-recommends mingw-w64
RUN apt-get install -y winetricks

ADD vc6.tar.xz /opt
# this overrides the watcom INCLUDE define. TODO(mkm): remove owsetenv above (breaks docker caching)
RUN echo 'export VC6=/vc6; export Path=Z:$VC6/bin; export INCLUDE=$VC6/include; export LIB=$VC6/lib' >>/root/.bashrc

# warmup wine
RUN wine dummy 2>/dev/null || true

ADD dobuild /usr/local/bin/

# misc tools to make it easier to work interactively within the image
RUN apt-get update; apt-get install -y file less zile vim

VOLUME ["/cesanta"]
ENTRYPOINT ["/usr/local/bin/dobuild", "-C", "/cesanta/v7", "ONDOCKER=1"]
