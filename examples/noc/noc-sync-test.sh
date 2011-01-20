#!/bin/sh

INJECTION_PROBABILITY="0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1"
DATA_PACKET_SPEEDUP="1 2 4"

echo "NoC sync test"

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
  echo "Kill data.db? (y/n)"
  read ANS
  if [ "$ANS" = "" -o "$ANS" = "yes" -o "$ANS" = "y" ]
  then
    echo Deleting database
    rm ../../data.db
  fi
fi

for injection_probability in $INJECTION_PROBABILITY
do
  for data_packet_speedup in $DATA_PACKET_SPEEDUP
  do
    echo Injection probability $injection_probability, data packet speedup $data_packet_speedup
    ../../waf --run "noc-sync-test --injection-probability=$injection_probability --data-packet-speedup=$data_packet_speedup"
  done
done

#mv ../../data.db .

for data_packet_speedup in $DATA_PACKET_SPEEDUP
do
  	INPUT="-${data_packet_speedup}-destination_specified"
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
	
	sqlite3 -noheader ../../data.db "$CMD" > noc-sync-test${INPUT}.data
	sed -i "s/|/   /" noc-sync-test${INPUT}.data
done

gnuplot noc-sync-test.gnuplot

echo "Done; data in noc-sync-test.data, plot in noc-sync-test.png"

