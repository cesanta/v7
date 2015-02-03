#/bin/sh

if true; then
(
    git log --pretty='format:%H;%ct' --no-merges a0fa567305daea5a146291..HEAD \
        | while read l; do
              sha=$(echo $l | awk -F';' '{print $1}');
              date=$(echo $l | awk -F';' '{print $2}');
              printf "%s " "${date}"
              git show ${sha}:tests/ecma_report.txt \
                  | awk '/tail/{tests++} /PASS/{pass++} END {printf "%.2lf\n", pass/tests*100}'
          done
) > tests/hist_ecma_report.csv
fi

gnuplot <<EOF
set terminal svg size 1280 780
set output 'tests/hist_ecma_report.svg'
set xdata time
set timefmt "%s"
set autoscale
set yrange [0:100]
plot 'tests/hist_ecma_report.csv' using 1:2 with linespoints pointtype 7 pointsize 0.5
EOF
