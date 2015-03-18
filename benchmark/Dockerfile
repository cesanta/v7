FROM cesanta/v7_benchmark_base:latest

COPY v7.* /v7/

RUN gcc -o /v7/v7 /v7/v7.c -DV7_EXE -O3 -lm && \
    strip /v7/v7

COPY *.js bench.sh render.gnuplot /v7/

WORKDIR /v7
ENTRYPOINT ["/v7/bench.sh"]
VOLUME ["/v7/graphs"]