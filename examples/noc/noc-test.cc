/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Systems and Networking, University of Augsburg, Germany
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Ciprian Radu <radu@informatik.uni-augsburg.de>
 */

// This example is based on the CSMA example csma-packet-socket.cc
//
// Network topology: 2D mesh
//
//
//

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/noc-module.h"
#include "ns3/mobility-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NocTest");

uint32_t numberOfNodes = 16;
uint32_t hSize = 4;

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Here, we will explicitly create four nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  for (unsigned int i = 0; i < numberOfNodes; ++i) {
    Ptr<NocNode> nocNode = CreateObject<NocNode> ();
    nodes.Add(nocNode);
  }
//  nodes.Create (numberOfNodes);

  // use a helper function to connect our nodes to the shared channel.
  NS_LOG_INFO ("Build Topology.");
  Ptr<NocHelper> noc = CreateObject<NocHelper> ();
  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  noc->SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer devs = noc->Install2DMesh (nodes, hSize);

  NS_LOG_INFO ("Create Applications.");
  NocApplicationHelper nocAppHelper (nodes, devs, hSize);
  nocAppHelper.SetAttribute("DataRate", DataRateValue(DataRate("50kb/s")));
  nocAppHelper.SetAttribute("TrafficPattern", EnumValue(NocApplication::UNIFORM_RANDOM));
  ApplicationContainer apps = nocAppHelper.Install (nodes.Get (0));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (1.0));

  nocAppHelper.SetAttribute("TrafficPattern", EnumValue(NocApplication::BIT_COMPLEMENT));
  apps = nocAppHelper.Install (nodes.Get (1));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (2.0));
 
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-test.tr file
  NS_LOG_INFO ("Configure Tracing.");
  std::ofstream os;
  os.open ("noc-test.tr", std::ios_base::binary | std::ios_base::out);
  noc->EnableAsciiAll (os);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  // TODO The simulation is not quite done. Unsent packets could still in the buffers.
  // The buffers should be emptied at the end of the simulation.
  // Note that the simulation is kept in motion by the injection of packets in the Noc
  // (done by the applications)

  os.close();

  return 0;
}
