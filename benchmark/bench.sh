#!/bin/bash

VMS="V7 DUK MUJS NODEJS"

NODEJS=node
which nodejs && NODEJS=nodejs

if [ -e /v7 ]; then
    V7=./v7
    DUK=./duk
    MUJS=./mujs
else
    V7=../v7
    DUK=duk
    MUJS=mujs
fi

TIME=/usr/bin/time
if [ -e /usr/local/bin/gtime ]; then
    TIME=/usr/local/bin/gtime
fi

function header() {
    echo -en "Title"
    for N in ${VMS}; do
        echo -ne "\t${N}"
    done
    echo
}

header >/tmp/v7-bench-cpu.data
header >/tmp/v7-bench-mem.data

for B in bench_*.js; do
    for N in ${VMS}; do
        eval V=\$${N}
        echo "Running ${B} with ${N}"
        ${TIME} -o /tmp/v7-bench-raw.data --format="\"${B}\"\t%U\t%M" ${V} ${B}
        awk '{print $2}' /tmp/v7-bench-raw.data >/tmp/v7-bench-cpu-${N}.data
        awk '{print $3}' /tmp/v7-bench-raw.data >/tmp/v7-bench-mem-${N}.data
    done

    S=$(echo ${B} | sed 's/bench_//' )
    (
        echo -n "\"${S}\""
        for N in ${VMS}; do
            echo -ne "\t"
            echo -n $(cat /tmp/v7-bench-cpu-${N}.data)
        done
        echo
    ) >>/tmp/v7-bench-cpu.data
    (
        echo -n "\"${S}\""
        for N in ${VMS}; do
            echo -ne "\t"
            echo -n $(cat /tmp/v7-bench-mem-${N}.data)
        done
        echo
    ) >>/tmp/v7-bench-mem.data

done

if [ -d graphs ]; then
    cp /tmp/v7-bench-{cpu,mem}.data graphs/
fi
