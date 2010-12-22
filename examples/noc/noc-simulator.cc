/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010
 *               Advanced Computer Architecture and Processing Systems (ACAPS),
 *               Lucian Blaga University of Sibiu, Romania
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
 * Author: Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *         http://webspace.ulbsibiu.ro/ciprian.radu/
 */

// Network on chip simulator
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
#include "ns3/topology-module.h"
#include "ns3/noc-application.h"
#include "ns3/noc-application-helper.h"
#include "ns3/noc-node.h"
#include "ns3/mobility-helper.h"
//#include "ns3/gtk-config-store.h"
#include "ns3/slb-load-router-component.h"
#include "ns3/so-load-router-component.h"
#include "ns3/noc-registry.h"
#include "ns3/integer.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/uinteger.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NocSimulator");

uint32_t numberOfNodes = 16;
uint32_t hSize = 4;

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // set the global parameters
  NocRegistry::GetInstance ()->SetAttribute ("DataPacketSpeedup", IntegerValue (2));

  // Here, we will explicitly create four nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  for (unsigned int i = 0; i < numberOfNodes; ++i) {
    Ptr<NocNode> nocNode = CreateObject<NocNode> ();
    nodes.Add (nocNode);
  }
//  nodes.Create (numberOfNodes);

  // use a helper function to connect our nodes to the shared channel.
  NS_LOG_INFO ("Build Topology.");
  Ptr<NocTopology> noc = CreateObject<NocIrvineMesh2D> ();
  noc->SetAttribute ("hSize", UintegerValue (hSize));
  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate ("50Mib/s")));
  noc->SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0)));
  noc->SetInQueue ("ns3::DropTailQueue",
      "Mode", EnumValue (DropTailQueue::PACKETS),
      "MaxPackets", UintegerValue (1));

  // install the topology
  noc->SetRouter ("ns3::IrvineLoadRouter");
  // WARNING setting properties for objects in this manner means that all the created objects
  // will refer to the *same* object
  //
  // example: all routers will use the *same* load component; this is obviously incorrect
  // we therefore can't do something like this:
  //
  // Ptr<LoadRouterComponent> loadComponent = CreateObject<SlbLoadRouterComponent> ();

//  noc->SetRouterAttribute ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SlbLoadRouterComponent")));
  noc->SetRouterAttribute ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SoLoadRouterComponent")));

  // Do not forget about changing the routing protocol when changing the load router component

//  noc->SetRoutingProtocol ("ns3::XyRouting");
//  noc->SetRoutingProtocolAttribute ("RouteXFirst", BooleanValue (false));

//  noc->SetRoutingProtocol ("ns3::SlbRouting");
//  noc->SetRoutingProtocolAttribute ("LoadThreshold", IntegerValue (30));

  noc->SetRoutingProtocol ("ns3::SoRouting");

  noc->SetSwitchingProtocol ("ns3::WormholeSwitching");
//  noc->SetSwitchingProtocol ("ns3::SafSwitching");
//  noc->SetSwitchingProtocol ("ns3::VctSwitching");

  NetDeviceContainer devs = noc->Install (nodes);
  // done with installing the topology

  NS_LOG_INFO ("Create Applications.");

  for (unsigned int i = 0; i < nodes.GetN (); ++i)
    {
      NocApplicationHelper nocAppHelper (nodes, devs, hSize);
      nocAppHelper.SetAttribute ("NumberOfPackets", UintegerValue (8));
      nocAppHelper.SetAttribute ("DataRate", DataRateValue (DataRate("4096b/s")));
      nocAppHelper.SetAttribute ("TrafficPattern", EnumValue (NocApplication::BIT_MATRIX_TRANSPOSE));
//      nocAppHelper.SetAttribute ("Destination", UintegerValue (15)); // destination
      ApplicationContainer apps = nocAppHelper.Install (nodes.Get (i)); // source
      apps.Start (Seconds (0.0));
      apps.Stop (Seconds (10.0));
    }
 
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-simulator.tr file
  NS_LOG_INFO ("Configure Tracing.");
  Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper> ("noc-simulator.tr", std::ios_base::binary | std::ios_base::out);
  noc->EnableAsciiAll (stream);

//  GtkConfigStore configstore;
//  configstore.ConfigureAttributes();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  // TODO The simulation is not quite done. Unsent packets could still be in the buffers.
  // The buffers should be emptied at the end of the simulation.
  // Note that the simulation is kept in motion by the injection of packets in the Noc
  // (done by the applications)

  return 0;
}
