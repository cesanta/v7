all: run

run:
	./bench.sh

serve:
	./serve.sh

build: ../v7.c ../v7.h
	cp ../v7.[ch] .
	docker build -t cesanta/v7_benchmark .

build_serve:
	docker build -t cesanta/v7_benchmark_serve -f Dockerfile-serve .

build_base:
	docker build -t cesanta/v7_benchmark_base -f Dockerfile-base .

push:
	docker push cesanta/v7_benchmark

push_serve:
	docker push cesanta/v7_benchmark_serve

push_base:
	docker push cesanta/v7_benchmark_base
