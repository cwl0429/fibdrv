reset
set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci runtime'
set term png enhanced font 'Verdana,10'
set output 'plot_statistic_output.png'
set grid
plot [:][:] \
'plot_statistic_input' using 1:2 with linespoints linewidth 2 title "iterative",\
'' using 1:3 with linespoints linewidth 2 title "fast doubling"