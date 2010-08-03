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
#include "ns3/noc-application.h"
#include "ns3/noc-application-helper.h"
#include "ns3/noc-node.h"
#include "ns3/mobility-helper.h"
//#include "ns3/gtk-config-store.h"
#include "ns3/slb-load-router-component.h"

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
  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate ("50Mib/s")));
  noc->SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0)));
  noc->SetInQueue ("ns3::DropTailQueue",
      "Mode", EnumValue (DropTailQueue::PACKETS),
      // FIXME found problems with store-and-forward when buffer size is 1
      // could this be due to the fact that a message is made of 3 packets?
      "MaxPackets", UintegerValue (1000)); // using very big input channel buffers

  // install the topology
  ObjectFactory routerFactory;
  routerFactory.SetTypeId ("ns3::IrvineLoadRouter");
  // WARNING setting properties for objects in this manner means that all the created objects
  // will refer to the *same* object
  //
  // example: all routers will use the *same* load component; this is obviously incorrect
  // we therefore can't do something like this:
  //
  // Ptr<LoadRouterComponent> loadComponent = CreateObject<SlbLoadRouterComponent> ();
  routerFactory.Set("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SlbLoadRouterComponent")));

  ObjectFactory routingProtocolFactory;
//  routingProtocolFactory.SetTypeId ("ns3::XyRouting");
//  routingProtocolFactory.Set ("RouteXFirst", BooleanValue (false));
  routingProtocolFactory.SetTypeId ("ns3::SlbRouting");
  routingProtocolFactory.Set ("LoadThreshold", IntegerValue (30));

  ObjectFactory switchingProtocolFactory;
  switchingProtocolFactory.SetTypeId ("ns3::SafSwitching");

  NetDeviceContainer devs = noc->Install2DMeshIrvine (nodes, hSize,
      routerFactory,
      routingProtocolFactory,
      switchingProtocolFactory);
  // done with installing the topology

  NS_LOG_INFO ("Create Applications.");
  NocApplicationHelper nocAppHelper1 (nodes, devs, hSize);
  nocAppHelper1.SetAttribute("DataRate", DataRateValue(DataRate("4096b/s")));
  nocAppHelper1.SetAttribute("TrafficPattern", EnumValue(NocApplication::DESTINATION_SPECIFIED));
  nocAppHelper1.SetAttribute("Destination", UintegerValue (15)); // destination
  ApplicationContainer apps1 = nocAppHelper1.Install (nodes.Get (0)); // source
  apps1.Start (Seconds (0.0));
  apps1.Stop (Seconds (10.0));

  NocApplicationHelper nocAppHelper2 (nodes, devs, hSize);
  nocAppHelper2.SetAttribute("DataRate", DataRateValue(DataRate("4096b/s")));
  nocAppHelper2.SetAttribute("TrafficPattern", EnumValue(NocApplication::DESTINATION_SPECIFIED));
  nocAppHelper2.SetAttribute("Destination", UintegerValue (0)); // destination
  ApplicationContainer apps2 = nocAppHelper2.Install (nodes.Get (15)); // source
  apps2.Start (Seconds (0.0));
  apps2.Stop (Seconds (10.0));
 
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-test.tr file
  NS_LOG_INFO ("Configure Tracing.");
  std::ofstream os;
  os.open ("noc-test.tr", std::ios_base::binary | std::ios_base::out);
  noc->EnableAsciiAll (os);

//  GtkConfigStore configstore;
//  configstore.ConfigureAttributes();

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