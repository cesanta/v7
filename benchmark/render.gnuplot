V7 = "#99ffff"; DUK = "#4671d5"; MUJS = "#ff0000"; NODEJS = "#ffff00"
set auto x
set auto y
set style data histogram
set style histogram cluster gap 1
set style fill solid border -1
set boxwidth 0.9
set xtic scale 0

set terminal pngcairo size 800,600

set title "V7 CPU B_enchmarks" noenhanced 
set output 'bench-cpu.png'
plot '/tmp/v7-bench-cpu.data' using 2:xtic(1) ti col fc rgb V7, '' u 3 ti col fc rgb DUK, '' u 4 ti col fc rgb MUJS, '' u 5 ti col fc rgb NODEJS

set title "V7 MEM Benchmarks"
set output 'bench-mem.png'
plot '/tmp/v7-bench-mem.data' using 2:xtic(1) ti col fc rgb V7, '' u 3 ti col fc rgb DUK, '' u 4 ti col fc rgb MUJS, '' u 5 ti col fc rgb NODEJS
