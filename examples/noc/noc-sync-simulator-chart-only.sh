#!/bin/sh

INJECTION_PROBABILITY="0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1"
DATA_PACKET_SPEEDUP="1 2 4"
BUFFER_SIZE="9"
TRAFFIC_PATTERN="UniformRandom BitMatrixTranspose BitComplement BitReverse"

echo "NoC sync simulator (generate chart only)"

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

for data_packet_speedup in $DATA_PACKET_SPEEDUP
do
	for traffic_pattern in $TRAFFIC_PATTERN
	do
		INPUT="-${data_packet_speedup}-${BUFFER_SIZE}-${traffic_pattern}"
		CMD=" \
		SELECT E.INPUT, S1.VALUE AS 'LATENCY' \
		FROM SINGLETONS S1, EXPERIMENTS E \
		WHERE \
			E.INPUT LIKE '%${INPUT}' \
			AND \
			S1.RUN = E.RUN \
			AND \
			S1.VARIABLE = 'latency-average' \
		GROUP BY E.INPUT \
		ORDER BY ABS(E.INPUT) ASC \
		; \
		"
		
		sqlite3 -noheader ../../data.db "$CMD" > noc-sync-simulator${INPUT}.data
		sed -i "s/|/   /" noc-sync-simulator${INPUT}.data
	done
done

for traffic_pattern in $TRAFFIC_PATTERN
do
	echo "set terminal png" > temp.tmp
	#echo "set out \"noc-sync-simulator-${traffic_pattern}(scaled).png\"" >> temp.tmp
	echo "set out \"noc-sync-simulator-${traffic_pattern}.png\"" >> temp.tmp
	echo "set title \"${traffic_pattern} traffic in a 8x8 2D mesh\"" >> temp.tmp
	echo "set xlabel \"Packet injection probability\"" >> temp.tmp
	echo "set xrange [0:1]" >> temp.tmp
	echo "set ylabel \"Average packet latency (cycles)\"" >> temp.tmp
	#echo "set yrange [0:200]" >> temp.tmp
	
	echo "plot \"noc-sync-simulator-1-${BUFFER_SIZE}-${traffic_pattern}.data\" with lines title \"original\", \\
		 \"noc-sync-simulator-2-${BUFFER_SIZE}-${traffic_pattern}.data\" with lines title \"2x\", \\
		 \"noc-sync-simulator-4-${BUFFER_SIZE}-${traffic_pattern}.data\" with lines title \"4x\"" >> temp.tmp
	
	gnuplot temp.tmp
done

rm temp.tmp

echo "Done; data in noc-sync-simulator*.data, plot in noc-sync-simulator*.png"

