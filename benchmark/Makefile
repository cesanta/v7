all: run

run:
	./bench.sh

build: ../v7.c ../v7.h
	cp ../v7.[ch] .
	docker build -t cesanta/v7_benchmark .

push:
	docker push cesanta/v7_benchmark
