#!/bin/sh

exec docker run --rm -v $PWD/..:/cesanta -v $PWD/../vc6:/vc6 -t -i cesanta/v7_test "$@"
