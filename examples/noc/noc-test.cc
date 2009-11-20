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
  nodes.Create (numberOfNodes);

  PacketSocketHelper packetSocket;
  packetSocket.Install (nodes);

  // create the shared medium used by all NoC devices.
  NS_LOG_INFO ("Create channels.");
  Ptr<NocChannel> channel = CreateObject<NocChannel> ();

  // use a helper function to connect our nodes to the shared channel.
  NS_LOG_INFO ("Build Topology.");
  NocHelper noc;
  NetDeviceContainer devs = noc.Install2DMesh (nodes, hSize);

  NS_LOG_INFO ("Create Applications.");
  NocApplicationHelper nocAppHelper ("ns3::PacketSocketFactory", devs, hSize);
  ApplicationContainer apps = nocAppHelper.Install (nodes.Get (0));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (1.0));

//  apps = nocAppHelper.Install (nodes.Get (3));
//  apps.Start (Seconds (0.0));
//  apps.Stop (Seconds (2.0));
 
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-test.tr file
  NS_LOG_INFO ("Configure Tracing.");
  std::ofstream os;
  os.open ("noc-test.tr", std::ios_base::binary | std::ios_base::out);
  noc.EnableAsciiAll (os);

//  // Setup mobility - static grid topology
//  MobilityHelper mobility;
//  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
//                                 "MinX", DoubleValue (0.0),
//                                 "MinY", DoubleValue (0.0),
//                                 "DeltaX", DoubleValue (10),
//                                 "DeltaY", DoubleValue (10),
//                                 "GridWidth", UintegerValue (2),
//                                 "LayoutType", StringValue ("RowFirst"));
//  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//  mobility.Install (nodes);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  os.close();

  return 0;
}
