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
#include "ns3/noc-sync-application.h"
#include "ns3/noc-sync-application-helper.h"
#include "ns3/noc-node.h"
#include "ns3/mobility-helper.h"
//#include "ns3/gtk-config-store.h"
#include "ns3/slb-load-router-component.h"
#include "ns3/so-load-router-component.h"
#include "ns3/noc-registry.h"
#include "ns3/integer.h"
#include "ns3/stats-module.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NocSyncTest");

uint32_t numberOfNodes = 1024;
uint32_t hSize = 32;

int
main (int argc, char *argv[])
{
  NS_ASSERT_MSG (numberOfNodes % hSize == 0,
      "The number of nodes ("<< numberOfNodes
      <<") must be a multiple of the number of nodes on the horizontal axis ("
      << hSize << ")");

  double injectionProbability (1);
  int dataPacketSpeedup (4);

  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue<double> ("injection-probability", "The packet injection probability.", injectionProbability);
  cmd.AddValue<int> ("data-packet-speedup", "The speedup used for data packets (compared to head packets)", dataPacketSpeedup);
  cmd.Parse (argc, argv);

  // set the global parameters
  NocRegistry::GetInstance ()->SetAttribute ("DataPacketSpeedup", IntegerValue (dataPacketSpeedup));
  NocRegistry::GetInstance ()->SetAttribute ("GlobalClock", TimeValue (Seconds (1)));

  // Here, we will explicitly create four nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  for (unsigned int i = 0; i < numberOfNodes; ++i)
    {
      Ptr<NocNode> nocNode = CreateObject<NocNode> ();
      nodes.Add (nocNode);
    }
//  nodes.Create (numberOfNodes);

  // use a helper function to connect our nodes to the shared channel.
  NS_LOG_INFO ("Build Topology.");
  Ptr<NocHelper> noc = CreateObject<NocHelper> ();
  // Note that the channel attributes are not considered with a NocSyncApplication!
//  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate ("50Mib/s")));
//  noc->SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0)));
  noc->SetInQueue ("ns3::DropTailQueue",
      "Mode", EnumValue (DropTailQueue::PACKETS),
      "MaxPackets", UintegerValue (1)); // the in queue must have at least 1 packet

  // install the topology
  ObjectFactory routerFactory;
//  routerFactory.SetTypeId ("ns3::IrvineLoadRouter");
  routerFactory.SetTypeId ("ns3::IrvineRouter");
  // WARNING setting properties for objects in this manner means that all the created objects
  // will refer to the *same* object
  //
  // example: all routers will use the *same* load component; this is obviously incorrect
  // we therefore can't do something like this:
  //
  // Ptr<LoadRouterComponent> loadComponent = CreateObject<SlbLoadRouterComponent> ();
//  routerFactory.Set ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SlbLoadRouterComponent")));
//  routerFactory.Set ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SoLoadRouterComponent")));
  // Do not forget about changing the routing protocol when changing the load router component

  ObjectFactory routingProtocolFactory;
  routingProtocolFactory.SetTypeId ("ns3::XyRouting");
//  routingProtocolFactory.Set ("RouteXFirst", BooleanValue (false));

//  routingProtocolFactory.SetTypeId ("ns3::SlbRouting");
//  routingProtocolFactory.Set ("LoadThreshold", IntegerValue (30));

//  routingProtocolFactory.SetTypeId ("ns3::SoRouting");

  ObjectFactory switchingProtocolFactory;
  switchingProtocolFactory.SetTypeId ("ns3::WormholeSwitching");
//  switchingProtocolFactory.SetTypeId ("ns3::SafSwitching");
//  switchingProtocolFactory.SetTypeId ("ns3::VctSwitching");

  NetDeviceContainer devs = noc->Install2DMeshIrvine (nodes, hSize,
      routerFactory,
      routingProtocolFactory,
      switchingProtocolFactory);
  // done with installing the topology

  NS_LOG_INFO ("Create Applications.");
  NocSyncApplicationHelper nocSyncAppHelper1 (nodes, devs, hSize);
  nocSyncAppHelper1.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
  nocSyncAppHelper1.SetAttribute ("TrafficPattern", EnumValue (NocSyncApplication::DESTINATION_SPECIFIED));
  nocSyncAppHelper1.SetAttribute ("Destination", UintegerValue (15)); // destination
//  nocSyncAppHelper1.SetAttribute ("MaxPackets", UintegerValue (10));
  ApplicationContainer apps1 = nocSyncAppHelper1.Install (nodes.Get (0)); // source
  apps1.Start (Seconds (0.0));
  apps1.Stop (Seconds (10.0));

  NocSyncApplicationHelper nocSyncAppHelper2 (nodes, devs, hSize);
  nocSyncAppHelper2.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
  nocSyncAppHelper2.SetAttribute ("TrafficPattern", EnumValue (NocSyncApplication::DESTINATION_SPECIFIED));
  nocSyncAppHelper2.SetAttribute ("Destination", UintegerValue (0)); // destination
//  nocSyncAppHelper2.SetAttribute ("MaxPackets", UintegerValue (10));
  ApplicationContainer apps2 = nocSyncAppHelper2.Install (nodes.Get (3)); // source
  apps2.Start (Seconds (0.0));
  apps2.Stop (Seconds (10.0));
 
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-sync-test.tr file
// Tracing should be kept disabled for big simulations
  NS_LOG_INFO ("Configure Tracing.");
  Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper> ("noc-sync-test.tr", std::ios_base::binary | std::ios_base::out);
  noc->EnableAsciiAll (stream);

//  GtkConfigStore configstore;
//  configstore.ConfigureAttributes();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
