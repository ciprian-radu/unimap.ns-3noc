#!/bin/bash

# Each ns-3 NoC script must have a unique SCRIPT_ID (AKA experiment ID)
# This helps at identifying the simulation results in the database
#
# This script runs multiple applications and shows how much time the simulation required 

SCRIPT_ID="4"

# Do not use spaces for the values passed to ns-3 NoC as simulator parameters

EXPERIMENT="Simulation_runtime_on_a_2D_mesh_NoC_(UniMap_test)"
STRATEGY="mapping_file_path"
DESCRIPTION="global_clock=1GHz,router=FourWay,routing_protocol=XY,switching_mechanism=wormhole,no_data_flit_speedup"
AUTHOR="Ciprian_Radu"

#WARMUP_CYCLES="1000"
#SIMULATION_CYCLES="10000"

FLIT_SIZE="32"
NUMBER_OF_FLITS_PER_PACKET="10"
APPLICATION="consumer-mocsyn_bb consumer-mocsyn_manual"
declare MAPPING_FILE_PATH
MAPPING_FILE_PATH=(\
"../CTG-XML/xml/e3s/auto-indust-mocsyn.tgff/ctg-0+1+2+3/mapping-0+1+2+3_2_bb.xml" \
"../CTG-XML/xml/e3s/consumer-mocsyn.tgff/ctg-0+1/mapping-0+1_2_bb.xml" \
"../CTG-XML/xml/e3s/networking-mocsyn.tgff/ctg-0+1+2+3/mapping-0+1+2+3_2_bb.xml" \
"../CTG-XML/xml/e3s/office-automation-mocsyn.tgff/ctg-0/mapping-0_2_bb.xml" \
"../CTG-XML/xml/e3s/telecom-mocsyn.tgff/ctg-0+1+2+3+4+5+6+7+8/mapping-0+1+2+3+4+5+6+7+8_2_bb.xml" \
"../CTG-XML/xml/PIP/ctg-0/mapping-0_m_bb.xml" \
"../CTG-XML/xml/MPEG4/ctg-0/mapping-0_m_bb.xml" \
"../CTG-XML/xml/MWD/ctg-0/mapping-0_m_bb.xml" \
"../CTG-XML/xml/H.264/ctg-0/mapping-0_m_bb.xml" \
"../CTG-XML/xml/H.264/ctg-1/mapping-1_m_bb.xml" \
"../CTG-XML/xml/VOPD/ctg-0/mapping-0_m_bb.xml" \
"../CTG-XML/xml/VOPD/ctg-1/mapping-1_m_bb.xml" \
"../CTG-XML/xml/MMS/ctg-0/mapping-0_m_bb.xml" \
"../CTG-XML/xml/MMS/ctg-1/mapping-1_m_bb.xml" \
)
declare H_SIZE
H_SIZE=(5 4 4 3 6 3 4 4 4 4 4 4 4 5)
declare NODES
NODES=(25 12 16 6 30 9 12 12 16 16 16 12 16 25)

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

	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin/:/usr/local/lib/:${HOME}/workspace/ns3dev/build/debug/src/noc/orion
	export NS_LOG="ns-3NoCUniMap=level_info:*=level_error|prefix_func|prefix_time"
	cd $HOME/workspace/ns3dev/examples/noc

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
	COUNTER=0
	for mapping_file_path in ${MAPPING_FILE_PATH[@]}
	do
		echo "Number of flits per packet ${number_of_flits_per_packet}, mapping file path ${mapping_file_path}"
		INPUT="${number_of_flits_per_packet},${mapping_file_path}"
		RUN="experiment_id=${SCRIPT_ID},number_of_flits_per_packet=${number_of_flits_per_packet},mapping_file_path=${mapping_file_path}"
	    time ../../waf --run "ns-3-noc-unimap --experiment=$EXPERIMENT --strategy=$STRATEGY --input=$INPUT --description=$DESCRIPTION, --run=$RUN --author=$AUTHOR --mapping-file-path=${mapping_file_path} --nodes=${NODES[${COUNTER}]} --h-size=${H_SIZE[${COUNTER}]} --flit-size=${FLIT_SIZE} --flits-per-packet=${number_of_flits_per_packet}"
	   	COUNTER=$(($COUNTER+1))
	done	    	
done

echo "Done"

