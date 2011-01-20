#!/bin/bash

# Each ns-3 NoC script must have a unique SCRIPT_ID (AKA experiment ID)
# This helps at identifying the simulation results in the database
#
# If you run this script with the "co" parameter (chart only), then no simulation is performed,
# the data is considered to already be in the database. Only the charts are generated.

SCRIPT_ID="2"

# Do not use spaces for the values passed to ns-3 NoC as simulator parameters

EXPERIMENT="Average_packet_latency_evaluation_on_a_2D_4x4_mesh_Irvine_NoC_(buffer_size)"
STRATEGY="packet_injection_probability,buffer,traffic_pattern"
DESCRIPTION="Irvine_2D_mesh_size=4x4,global_clock=1GHz,router=Irvine,routing_protocol=XY,switching_mechanism=wormhole,number_of_flits_per_packet=9,no_data_flit_speedup,warmup_cycles=1000,simulation_cycles=10000"
AUTHOR="Ciprian_Radu"

NODES="16"
H_SIZE="4"
WARMUP_CYCLES="1000"
SIMULATION_CYCLES="10000"

INJECTION_PROBABILITY="0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1"
DATA_FLIT_SPEEDUP="1"
BUFFER_SIZE="2 3 4 5 6 7 8 9 10"
TRAFFIC_PATTERN="UniformRandom BitMatrixTranspose BitComplement BitReverse"

if [ "$1" != "co" ]
then
	echo "Starting the experiment '${EXPERIMENT}' with the ns-3 NoC simulator"

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

	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin/

	if [ -e ../../data.db ]
	then
	  echo "Kill data.db? (y/N)"
	  read ANS
	  if [ "$ANS" = "yes" -o "$ANS" = "y" -o "$ANS" = "Y" ]
	  then
	    echo Deleting database
	    rm ../../data.db
	  fi
	fi

	for injection_probability in $INJECTION_PROBABILITY
	do
	  for buffer_size in $BUFFER_SIZE
	  do
		for traffic_pattern in $TRAFFIC_PATTERN
		do
	    	echo Injection probability $injection_probability, buffer size $buffer_size, traffic pattern $traffic_pattern
		INPUT="${injection_probability},${buffer_size},${traffic_pattern}"
		RUN="experiment_id=${SCRIPT_ID},4x4_Irvine_2D_mesh,packet_injection_probability=${injection_probability},buffer_size=${buffer_size},traffic_pattern=${traffic_pattern}"
	    	time ../../waf --run "ns-3-noc --experiment=$EXPERIMENT --strategy=$STRATEGY --input=$INPUT --description=$DESCRIPTION, --run=$RUN --author=$AUTHOR --nodes=$NODES --h-size=$H_SIZE --injection-probability=$injection_probability --data-packet-speedup=$DATA_FLIT_SPEEDUP --buffer-size=$buffer_size --traffic-pattern=$traffic_pattern --warmup-cycles=$WARMUP_CYCLES --simulation-cycles=$SIMULATION_CYCLES"
		done
	  done
	done
fi

echo "Generating charts"

for buffer_size in $BUFFER_SIZE
do
	for traffic_pattern in $TRAFFIC_PATTERN
	do
		INPUT_DB="${buffer_size},${traffic_pattern}"
		INPUT_FILE="${buffer_size}-${traffic_pattern}"
		CMD=" \
		SELECT E.INPUT, S.VALUE AS 'LATENCY' \
		FROM SINGLETONS S, EXPERIMENTS E \
		WHERE \
			E.INPUT LIKE '%${INPUT_DB}' \
			AND \
			S.RUN LIKE 'experiment_id=${SCRIPT_ID},4x4_Irvine_2D_mesh,packet_injection_probability=%,buffer_size=%,traffic_pattern=%' \
			AND \
			S.RUN = E.RUN \
			AND \
			S.VARIABLE = 'latency-average' \
		GROUP BY E.INPUT \
		ORDER BY ABS(E.INPUT) ASC \
		; \
		"
		
		sqlite3 -noheader ../../data.db "$CMD" > ns-3-noc-${SCRIPT_ID}-${INPUT_FILE}.data
		sed -i "s/|/   /" ns-3-noc-${SCRIPT_ID}-${INPUT_FILE}.data
	done
done

for traffic_pattern in $TRAFFIC_PATTERN
do
	echo "set terminal png" > temp.tmp
	echo "set out \"ns-3-noc-${SCRIPT_ID}-${traffic_pattern}.png\"" >> temp.tmp
	echo "set title \"${traffic_pattern} traffic in a 2D 4x4 mesh Irvine NoC\"" >> temp.tmp
	echo "set xlabel \"Packet injection rate (packets/cycle)\"" >> temp.tmp
	echo "set xrange [0:1]" >> temp.tmp
	echo "set ylabel \"Average packet latency (cycles)\"" >> temp.tmp
	#echo "set yrange [0:200]" >> temp.tmp
	
	echo "plot \"ns-3-noc-${SCRIPT_ID}-2-${traffic_pattern}.data\" with lines title \"buffer size = 2\", \\
		   \"ns-3-noc-${SCRIPT_ID}-3-${traffic_pattern}.data\" with lines title \"buffer size = 3\", \\
		   \"ns-3-noc-${SCRIPT_ID}-4-${traffic_pattern}.data\" with lines title \"buffer size = 4\", \\
		   \"ns-3-noc-${SCRIPT_ID}-5-${traffic_pattern}.data\" with lines title \"buffer size = 5\", \\
		   \"ns-3-noc-${SCRIPT_ID}-6-${traffic_pattern}.data\" with lines title \"buffer size = 6\", \\
		   \"ns-3-noc-${SCRIPT_ID}-7-${traffic_pattern}.data\" with lines title \"buffer size = 7\", \\
		   \"ns-3-noc-${SCRIPT_ID}-8-${traffic_pattern}.data\" with lines title \"buffer size = 8\"" >> temp.tmp
	
	gnuplot temp.tmp
done

rm temp.tmp

echo "Done; data in ns-3-noc-${SCRIPT_ID}*.data, plot in ns-3-noc-${SCRIPT_ID}*.png"

