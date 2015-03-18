all: run

run:
	./bench.sh

build: ../v7.c ../v7.h
	cp ../v7.[ch] .
	docker build -t cesanta/v7_benchmark .

build_base:
	docker build -t cesanta/v7_benchmark_base -f Dockerfile-base .

push:
	docker push cesanta/v7_benchmark

push_base:
	docker push cesanta/v7_benchmark_base
