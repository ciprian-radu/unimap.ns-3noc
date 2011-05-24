#!/bin/bash

# Launcher script for ns-e NoC (UniMap)

echo "UniMap components are expected to be placed in workspace, a directory from your home directory"
echo "ns-3 NoC is expected to be in the ns3dev directory"
echo "CTG-XML is expected to be in the CTG-XML directory"
echo "NoC-XML is expected to be in the NoC-XML directory"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bin/:/usr/local/lib/:~/workspace/ns3dev/build/debug:~/workspace/ns3dev/build/debug/src/noc/orion
export NS_LOG="ns-3NoCUniMap=level_info:*=level_error|prefix_func|prefix_time"
cd ~/workspace/ns3dev/build/debug/examples/noc
echo "Running ns-3-noc-unimap with arguments: " $@
time ./ns-3-noc-unimap $@