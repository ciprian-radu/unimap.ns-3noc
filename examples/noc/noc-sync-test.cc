/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *               						Lucian Blaga University of Sibiu, Romania
 *               - Systems and Networking, University of Augsburg, Germany
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

// This example is based on the CSMA example csma-packet-socket.cc
//
// Network topology: 2D torus
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
#include "ns3/noc-sync-application.h"
#include "ns3/noc-sync-application-helper.h"
#include "ns3/noc-node.h"
#include "ns3/mobility-helper.h"
//#include "ns3/gtk-config-store.h"
#include "ns3/slb-load-router-component.h"
#include "ns3/so-load-router-component.h"
#include "ns3/noc-registry.h"
#include "ns3/integer.h"
#include "ns3/boolean.h"
#include "ns3/stats-module.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/uinteger.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NocSyncTest");

uint32_t numberOfNodes = 8;
uint32_t hSize = 2;
uint32_t vSize = 2;

int
main (int argc, char *argv[])
{
  NS_ASSERT_MSG (numberOfNodes % hSize == 0,
      "The number of nodes ("<< numberOfNodes
      <<") must be a multiple of the number of nodes on the horizontal axis ("
      << hSize << ")");

  NS_ASSERT_MSG (numberOfNodes % vSize == 0,
        "The number of nodes ("<< numberOfNodes
        <<") must be a multiple of the number of nodes on the vertical axis ("
        << hSize << ")");

  double injectionProbability (1);
  int dataPacketSpeedup (1);
  Time globalClock = PicoSeconds (1000); // 1 ns -> NoC @ 1GHz

  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue<double> ("injection-probability", "The packet injection probability.", injectionProbability);
  cmd.AddValue<int> ("data-packet-speedup", "The speedup used for data packets (compared to head packets)", dataPacketSpeedup);
  cmd.Parse (argc, argv);

  // set the global parameters
  NocRegistry::GetInstance ()->SetAttribute ("DataPacketSpeedup", IntegerValue (dataPacketSpeedup));
  NocRegistry::GetInstance ()->SetAttribute ("GlobalClock", TimeValue (globalClock));

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
  // Ptr<NocTopology> noc = CreateObject<NocMesh2D> ();

  // Ptr<NocTopology> noc = CreateObject<NocIrvineMesh2D> ();

  // Ptr<NocTopology> noc = CreateObject<NocTorus2D> ();

  // Ptr<NocTopology> noc = CreateObject<NocMesh3D> ();
  // noc->SetAttribute ("hSize", UintegerValue (hSize));
  // noc->SetAttribute ("vSize", UintegerValue (vSize));

  // Ptr<NocTopology> noc = CreateObject<NocTorus3D> ();

  int64_t dimensions = 3;
  vector<Ptr<NocValue> > size(dimensions);
  NocRegistry::GetInstance ()->SetAttribute ("NoCDimensions", IntegerValue (dimensions));
  size.at (0) = CreateObject<NocValue> (hSize);
  size.at (1) = CreateObject<NocValue> (vSize);
  size.at (2) = CreateObject<NocValue> (numberOfNodes / hSize / vSize);
  Ptr<NocTopology> noc = CreateObject<NocMeshND> (size);

  uint32_t flitSize = 8 * 6; // 3 bytes
  NocRegistry::GetInstance ()->SetAttribute ("FlitSize", IntegerValue (flitSize));

  // set channel bandwidth to 1 flit / network clock
  // the channel's bandwidth is obviously expressed in bits / s
  // however, in order to avoid losing precision, we work with PicoSeconds (instead of Seconds)
  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate ((uint64_t) (1e12 * (flitSize)
      / globalClock.GetPicoSeconds ()))));
  // the channel has no propagation delay
  noc->SetChannelAttribute ("Delay", TimeValue (PicoSeconds (10)));

  //  noc->SetChannelAttribute ("FullDuplex", BooleanValue (false));
  //  noc->SetChannelAttribute ("Length", DoubleValue (10)); // 10 micro-meters channel length
  noc->SetInQueue ("ns3::DropTailQueue", "Mode", EnumValue (DropTailQueue::PACKETS), "MaxPackets", UintegerValue (1)); // the in queue must have at least 1 packet

  // install the topology

  noc->SetRouter ("ns3::FourWayRouter");
  //  noc->SetRouter ("ns3::FourWayLoadRouter");
  //  noc->SetRouter ("ns3::IrvineLoadRouter");
  //  noc->SetRouter ("ns3::IrvineRouter");

  // noc->SetRouterAttribute ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SlbLoadRouterComponent")));
  // noc->SetRouterAttribute ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SoLoadRouterComponent")));
  // Do not forget about changing the routing protocol when changing the load router component

  // noc->SetRoutingProtocol ("ns3::XyRouting");
  // noc->SetRoutingProtocolAttribute ("RouteXFirst", BooleanValue (false));

//   noc->SetRoutingProtocol ("ns3::XyzRouting");
//   noc->SetRoutingProtocolAttribute ("RouteXFirst", BooleanValue (true));
//   noc->SetRoutingProtocolAttribute ("RouteXSecond", BooleanValue (true));
//   noc->SetRoutingProtocolAttribute ("RouteYFirst", BooleanValue (false));
//   noc->SetRoutingProtocolAttribute ("RouteYSecond", BooleanValue (true));

  // noc->SetRoutingProtocol ("ns3::SlbRouting");
  // noc->SetRoutingProtocolAttribute ("LoadThreshold", IntegerValue (30));

  // noc->SetRoutingProtocol ("ns3::SoRouting");

   noc->SetRoutingProtocol ("ns3::DorRouting");

  noc->SetSwitchingProtocol ("ns3::WormholeSwitching");
  // noc->SetSwitchingProtocol ("ns3::SafSwitching");
  // noc->SetSwitchingProtocol ("ns3::VctSwitching");

  NetDeviceContainer devs = noc->Install (nodes);
  NocRegistry::GetInstance ()->SetAttribute ("NoCTopology", PointerValue (noc));
  // done with installing the topology

  uint64_t packetLength = 2; // flits per packet

  NS_LOG_INFO ("Create Applications.");
  NocSyncApplicationHelper nocSyncAppHelper1 (nodes, devs, size);
  nocSyncAppHelper1.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
  nocSyncAppHelper1.SetAttribute ("TrafficPattern", EnumValue (NocSyncApplication::DESTINATION_SPECIFIED));
  nocSyncAppHelper1.SetAttribute ("Destination", UintegerValue (1)); // destination
  nocSyncAppHelper1.SetAttribute ("NumberOfFlits", UintegerValue (packetLength));
  ApplicationContainer apps1 = nocSyncAppHelper1.Install (nodes.Get (6)); // source
  apps1.Start (Seconds (0.0));
  apps1.Stop (Scalar (10) * globalClock);

//  NocSyncApplicationHelper nocSyncAppHelper2 (nodes, devs, size);
//  nocSyncAppHelper2.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
//  nocSyncAppHelper2.SetAttribute ("TrafficPattern", EnumValue (NocSyncApplication::DESTINATION_SPECIFIED));
//  nocSyncAppHelper2.SetAttribute ("Destination", UintegerValue (2)); // destination
//  nocSyncAppHelper2.SetAttribute ("NumberOfFlits", UintegerValue (packetLength));
//  ApplicationContainer apps2 = nocSyncAppHelper2.Install (nodes.Get (10)); // source
//  apps2.Start (Seconds (0.0));
//  apps2.Stop (Scalar (10) * globalClock);

//  NocSyncApplicationHelper nocSyncAppHelper3 (nodes, devs, size);
//  nocSyncAppHelper3.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
//  nocSyncAppHelper3.SetAttribute ("TrafficPattern", EnumValue (NocSyncApplication::DESTINATION_SPECIFIED));
//  nocSyncAppHelper3.SetAttribute ("Destination", UintegerValue (7)); // destination
//  nocSyncAppHelper3.SetAttribute ("NumberOfFlits", UintegerValue (packetLength));
//  ApplicationContainer apps3 = nocSyncAppHelper3.Install (nodes.Get (5)); // source
//  apps3.Start (Seconds (0.0));
//  apps3.Stop (Scalar (10) * globalClock);
//
//  NocSyncApplicationHelper nocSyncAppHelper4 (nodes, devs, size);
//  nocSyncAppHelper4.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
//  nocSyncAppHelper4.SetAttribute ("TrafficPattern", EnumValue (NocSyncApplication::DESTINATION_SPECIFIED));
//  nocSyncAppHelper4.SetAttribute ("Destination", UintegerValue (5)); // destination
//  nocSyncAppHelper4.SetAttribute ("NumberOfFlits", UintegerValue (packetLength));
//  ApplicationContainer apps4 = nocSyncAppHelper4.Install (nodes.Get (7)); // source
//  apps4.Start (Seconds (0.0));
//  apps4.Stop (Scalar (10) * globalClock);

  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-sync-test.tr file
  // Tracing should be kept disabled for big simulations
  NS_LOG_INFO ("Configure Tracing.");
  Ptr<OutputStreamWrapper> stream =
      Create<OutputStreamWrapper> ("noc-sync-test.tr", std::ios_base::binary | std::ios_base::out);
  noc->EnableAsciiAll (stream);

  //  GtkConfigStore configstore;
  //  configstore.ConfigureAttributes();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();

//  NS_LOG_INFO ("NoC dynamic power: " << noc->GetDynamicPower () << " W");
//  NS_LOG_INFO ("NoC leakage power: " << noc->GetLeakagePower () << " W");
//  NS_LOG_INFO ("NoC total power: " << noc->GetTotalPower () << " W");
//  NS_LOG_INFO ("NoC area: " << noc->GetArea () << " um^2");

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
