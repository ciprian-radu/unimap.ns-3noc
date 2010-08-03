#set terminal postscript portrait enhanced lw 2 "Helvetica" 14
set terminal png

#set size 1.0, 0.66

#-------------------------------------------------------
set out "noc-sync-test.png"
set title "Destination specified traffic in 4x4 mesh"
set xlabel "Traffic (flit/node/cycle)"
set xrange [0:1]
set ylabel "Average latency (cycles)"
set yrange [0:10]

plot "noc-sync-test-1-destination_specified.data" with lines title "data packet speedup = 1", \
	 "noc-sync-test-2-destination_specified.data" with lines title "data packet speedup = 2", \
	 "noc-sync-test-4-destination_specified.data" with lines title "data packet speedup = 4"
