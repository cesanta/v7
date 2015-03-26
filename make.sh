#!/bin/sh

exec docker run --rm -v $PWD/..:/cesanta -t -i cesanta/v7_test "$@"
