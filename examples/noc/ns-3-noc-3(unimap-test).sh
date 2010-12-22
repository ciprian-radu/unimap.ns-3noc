#!/bin/bash

# Each ns-3 NoC script must have a unique SCRIPT_ID (AKA experiment ID)
# This helps at identifying the simulation results in the database
#
# If you run this script with the "co" parameter (chart only), then no simulation is performed,
# the data is considered to already be in the database. Only the charts are generated.

SCRIPT_ID="3"

# Do not use spaces for the values passed to ns-3 NoC as simulator parameters

EXPERIMENT="Average_packet_latency_evaluation_on_a_2D_4x4_mesh_NoC_(UniMap_test)"
STRATEGY="number_of_flits_per_packet,mapping_file_path"
DESCRIPTION="2D_mesh_size=4x4,global_clock=1GHz,router=FourWay,routing_protocol=XY,switching_mechanism=wormhole,no_data_flit_speedup"
AUTHOR="Ciprian_Radu"

NODES="16"
H_SIZE="4"
#WARMUP_CYCLES="1000"
#SIMULATION_CYCLES="10000"

FLIT_SIZE="32"
NUMBER_OF_FLITS_PER_PACKET="2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20"
#NUMBER_OF_FLITS_PER_PACKET="10"
APPLICATION="consumer-mocsyn_bb consumer-mocsyn_manual"
declare MAPPING_FILE_PATH
MAPPING_FILE_PATH=("../CTG-XML/xml/e3s/consumer-mocsyn.tgff/ctg-0/mapping-0_2_bb.xml" "../CTG-XML/xml/e3s/consumer-mocsyn.tgff/ctg-0/mapping-0_2_manual.xml")

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

	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin/:/usr/local/lib/

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

	for number_of_flits_per_packet in $NUMBER_OF_FLITS_PER_PACKET
	do
		for mapping_file_path in ${MAPPING_FILE_PATH[@]}
		do
			echo "Number of flits per packet ${number_of_flits_per_packet}, mapping file path ${mapping_file_path}"
			INPUT="${number_of_flits_per_packet},${mapping_file_path}"
			RUN="experiment_id=${SCRIPT_ID},4x4_2D_mesh,number_of_flits_per_packet=${number_of_flits_per_packet},mapping_file_path=${mapping_file_path}"
		    	time ../../waf --run "ns-3-noc-unimap --experiment=$EXPERIMENT --strategy=$STRATEGY --input=$INPUT --description=$DESCRIPTION, --run=$RUN --author=$AUTHOR --mapping-file-path=${mapping_file_path} --nodes=$NODES --h-size=$H_SIZE --flit-size=${FLIT_SIZE} --flits-per-packet=${number_of_flits_per_packet}"
		done	    	
	done
fi

echo "Generating charts"

COUNTER=0
for application in $APPLICATION
do
	INPUT_DB=${MAPPING_FILE_PATH[${COUNTER}]}
	INPUT_FILE="${application}"
	CMD=" \
		SELECT E.INPUT, S.VALUE AS 'LATENCY' \
		FROM SINGLETONS S, EXPERIMENTS E \
		WHERE \
			E.INPUT LIKE '%,${INPUT_DB}' \
			AND \
			S.RUN LIKE 'experiment_id=${SCRIPT_ID},4x4_2D_mesh,number_of_flits_per_packet=%,mapping_file_path=$INPUT_DB' \
			AND \
			S.RUN = E.RUN \
			AND \
			S.VARIABLE = 'latency-average' \
		GROUP BY E.INPUT \
		ORDER BY ABS(E.INPUT) ASC \
		; \
		"
	
	echo $CMD
	sqlite3 -noheader ../../data.db "$CMD" > ns-3-noc-unimap-${SCRIPT_ID}-${INPUT_FILE}.data
	sed -i "s/|/   /" ns-3-noc-unimap-${SCRIPT_ID}-${INPUT_FILE}.data
	
	COUNTER=$(($COUNTER+1))
done

for application in $APPLICATION
do
	echo "set terminal png" > temp.tmp
	echo "set out \"ns-3-noc-${SCRIPT_ID}-${application}.png\"" >> temp.tmp
	echo "set title \"${application} traffic in a 2D 4x4 mesh NoC\"" >> temp.tmp
	echo "set xlabel \"Flits per packet\"" >> temp.tmp
	#echo "set xrange [0:1]" >> temp.tmp
	echo "set ylabel \"Average packet latency (cycles)\"" >> temp.tmp
	#echo "set yrange [0:200]" >> temp.tmp
	
	echo "plot \"ns-3-noc-unimap-${SCRIPT_ID}-${application}.data\" with lines title \"${application}\"" >> temp.tmp
	
	gnuplot temp.tmp
done

rm temp.tmp

echo "Done; data in ns-3-noc-${SCRIPT_ID}*.data, plot in ns-3-noc-${SCRIPT_ID}*.png"

