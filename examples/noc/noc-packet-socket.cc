/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

// This example is based on the CSMA example csma-packet-socket.cc
//
// Network topology
//
//       n0    n1   n2   n3
//       |     |    |    |
//     =====================
//
// - Tracing of queues and packet receptions to file "noc-packet-socket-sink.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/noc-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NoCPacketSocketExample");

std::ofstream g_os;

static void
SinkRx (std::string path, Ptr<const Packet> p, const Address &address)
{
  g_os << path << " " << p->GetSize () << std::endl;
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  g_os.open ("noc-packet-socket-sink.tr",std::ios_base::binary | std::ios_base::out);

  // Here, we will explicitly create four nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (4);

  PacketSocketHelper packetSocket;
  packetSocket.Install (nodes);

  // create the shared medium used by all NoC devices.
  NS_LOG_INFO ("Create channels.");
  Ptr<NocChannel> channel = CreateObject<NocChannel> ();

  // use a helper function to connect our nodes to the shared channel.
  NS_LOG_INFO ("Build Topology.");
  NocHelper noc;
  NetDeviceContainer devs = noc.Install (nodes, channel);

  for (int i = 0; i < 4; ++i) {
    std::cout << "(MAC) address of node " << i << " is " << devs.Get(i)->GetAddress() << std::endl;
  }

  NS_LOG_INFO ("Create Applications.");
  // Create the OnOff application to send raw datagrams
  PacketSocketAddress socket;

  // create an On Off application, on node 0, which sends messages to node 1
  socket.SetSingleDevice(devs.Get (0)->GetIfIndex ()); // assign this socket to node 0
  socket.SetPhysicalAddress (devs.Get (1)->GetAddress ());
//  socket.SetProtocol (2); // I don't know yet what this could be for
  NocOnOffHelper nocOnoff ("ns3::PacketSocketFactory", Address (socket));
  nocOnoff.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1.0)));
  nocOnoff.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0.0)));
  ApplicationContainer apps = nocOnoff.Install (nodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  // create an On Off application, on node 3, which sends messages to node 0
  socket.SetSingleDevice (devs.Get (3)->GetIfIndex ());  // assign this socket to node 3
  socket.SetPhysicalAddress (devs.Get (0)->GetAddress ());
//  socket.SetProtocol (3); // I don't know yet what this could be for
  nocOnoff.SetAttribute ("Remote", AddressValue (socket)); // this is just to change the destination address (from node 1 to 0) because we use the same instance of NoC On Off application
  nocOnoff.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0.0)));
  apps = nocOnoff.Install (nodes.Get (3));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  PacketSinkHelper sink = PacketSinkHelper ("ns3::PacketSocketFactory", socket);
  apps = sink.Install (nodes.Get (0));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (20.0));

  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                   MakeCallback (&SinkRx));
 
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-packet-socket.tr file
  NS_LOG_INFO ("Configure Tracing.");
  std::ofstream os;
  os.open ("noc-packet-socket.tr", std::ios_base::binary | std::ios_base::out);
  noc.EnableAsciiAll (os);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  g_os.close ();
  os.close();

  return 0;
}
