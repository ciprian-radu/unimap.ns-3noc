#!/bin/bash

# Launcher script for ns-e NoC (UniMap)

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin/:/usr/local/lib/:/home/cradu/workspace/ns3dev/build/debug:/home/cradu/workspace/ns3dev/build/debug/src/noc/orion
export NS_LOG="ns-3NoCUniMap=level_info:*=level_error|prefix_func|prefix_time"
cd /home/cradu/workspace/ns3dev/build/debug/examples/noc
echo $@
time ./ns-3-noc-unimap $@