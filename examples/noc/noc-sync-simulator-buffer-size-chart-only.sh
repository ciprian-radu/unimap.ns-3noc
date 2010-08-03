#!/bin/sh

INJECTION_PROBABILITY="0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1"
DATA_PACKET_SPEEDUP="1"
BUFFER_SIZE="2 3 4 5 6 7 8"
TRAFFIC_PATTERN="UniformRandom BitMatrixTranspose BitComplement BitReverse"

echo "NoC sync simulator"

pCheck=`which sqlite3`
if [ -z "$pCheck" ]
then
  echo "ERROR: This script requires sqlite3."
  exit 255
fi

pCheck=`which gnuplot`
if [ -z "$pCheck" ]
then
  echo "ERROR: This script requires gnuplot."
  exit 255
fi

pCheck=`which sed`
if [ -z "$pCheck" ]
then
  echo "ERROR: This script requires sed."
  exit 255
fi

for traffic_pattern in $TRAFFIC_PATTERN
do
	echo "set terminal png" > temp.tmp
	echo "set out \"noc-sync-simulator-${traffic_pattern}(scaled).png\"" >> temp.tmp
	echo "set title \"${traffic_pattern} traffic in 4x4 mesh\"" >> temp.tmp
	echo "set xlabel \"Packet injection rate (packets/cycle)\"" >> temp.tmp
	echo "set xrange [0:1]" >> temp.tmp
	echo "set ylabel \"Average packet latency (cycles)\"" >> temp.tmp
	echo "set yrange [0:200]" >> temp.tmp
	
	echo "plot \"noc-sync-simulator-${DATA_PACKET_SPEEDUP}-2-${traffic_pattern}.data\" with lines title \"buffer size = 2\", \\
		 \"noc-sync-simulator-${DATA_PACKET_SPEEDUP}-3-${traffic_pattern}.data\" with lines title \"buffer size = 3\", \\
		 \"noc-sync-simulator-${DATA_PACKET_SPEEDUP}-4-${traffic_pattern}.data\" with lines title \"buffer size = 4\", \\
		 \"noc-sync-simulator-${DATA_PACKET_SPEEDUP}-5-${traffic_pattern}.data\" with lines title \"buffer size = 5\", \\
		 \"noc-sync-simulator-${DATA_PACKET_SPEEDUP}-6-${traffic_pattern}.data\" with lines title \"buffer size = 6\", \\
		 \"noc-sync-simulator-${DATA_PACKET_SPEEDUP}-7-${traffic_pattern}.data\" with lines title \"buffer size = 7\", \\
		 \"noc-sync-simulator-${DATA_PACKET_SPEEDUP}-8-${traffic_pattern}.data\" with lines title \"buffer size = 8\"" >> temp.tmp
	
	gnuplot temp.tmp
done

rm temp.tmp

echo "Done; data in noc-sync-simulator*.data, plot in noc-sync-simulator*.png"

