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

// Network on chip simulator. The network is synchronized, i.e. it works with based on a clock frequency.
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
#include <map>
#include "ns3/output-stream-wrapper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NocSyncSimulator");

uint32_t numberOfNodes = 64;
uint32_t hSize = 8;

// parameters path and packet are required by the Trace Source Accesor
void MessageInjectedCallback (Ptr<CounterCalculator<> > calc, std::string path, Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Message injected counted in the " << calc->GetKey () << " statistic");

  calc->Update();
}

void PacketInjectedCallback (Ptr<CounterCalculator<> > calc, std::string path, Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Packet injected counted in the " << calc->GetKey () << " statistic");

  calc->Update();
}

/**
 * Simple data structure for keeping the latency times of a message
 */
class LatencyTimes
{
public:

  /** the time when the head of the message is injected into the network */
  Time m_startTime;

  /** the time when the tail of the message reaches the destination */
  Time m_endTime;
};

/** keeps the latency time of each fully received message */
std::map<uint32_t, LatencyTimes> g_latencyOfMessages;

/**
 * Each time a packet is received at a node, this function gets invoked.
 *
 * \param dummy dummy parameter, not actually used (required by MakeBOundCall (...))
 * \param path the path with the receiving net device
 * \param packet the received packet
 */
void PacketReceivedCallback (int dummy, std::string path, Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Packet with UID " << packet->GetUid () << " received");

  LatencyTimes latencyTimes;

  NocHeader header;
  packet->PeekHeader (header);

  uint32_t uid;
  NocPacketTag tag;
  packet->PeekPacketTag (tag);
  NocPacket::Type packetType = tag.GetPacketType ();
  switch (packetType) {
    case NocPacket::HEAD:
      NS_LOG_DEBUG ("Packet type is head");
      latencyTimes = g_latencyOfMessages[packet->GetUid ()];
      latencyTimes.m_startTime = tag.GetInjectionTime();
      break;
    case NocPacket::DATA:
      NS_LOG_DEBUG ("Packet type is data");
      break;
    case NocPacket::TAIL:
      NS_LOG_DEBUG ("Packet type is tail");
      uid = tag.GetPacketHeadUid ();
      latencyTimes = g_latencyOfMessages[uid];
      latencyTimes.m_endTime = tag.GetReceiveTime ();
      break;
    case NocPacket::UNKNOWN:
    default:
      NS_LOG_DEBUG ("Packet type is unknown");
      break;
  }

  if (NocPacket::HEAD == packetType)
    {
      g_latencyOfMessages[packet->GetUid ()] = latencyTimes;
    }
  if (NocPacket::TAIL == packetType)
    {
      uid = tag.GetPacketHeadUid ();
      g_latencyOfMessages[uid] = latencyTimes;
    }

  NS_LOG_LOGIC ("Packet " << *packet << " has injection time " << tag.GetInjectionTime()
      << " and receive time " << tag.GetReceiveTime ());
  NS_ASSERT (tag.GetReceiveTime ().GetNanoSeconds () - tag.GetInjectionTime().GetNanoSeconds () >= 0);
}

/**
 * Computes the latency of each fully received message.
 *
 * The latency of a network is the time required for a packet to traverse the network,
 * from the time the head of the packet arrives at the input port to the time the tail of
 * the packet departs the output port.
 *
 * \param the latency calculator
 */
void ComputeLatenciesOfMessages (Ptr<TimeMinMaxAvgTotalCalculator > calc)
{
  NS_LOG_FUNCTION_NOARGS ();

  std::map<uint32_t, LatencyTimes>::iterator it;
  for (it = g_latencyOfMessages.begin (); it != g_latencyOfMessages.end (); it++)
    {
      LatencyTimes lt = it->second;
      if (lt.m_endTime.IsZero ())
        {
          NS_LOG_LOGIC ("The message with the head with UID " << it->first << " has zero end time. "
              "This means the tail was not received and therefore, "
              "the latency of this message is not considered.");
        }
      else
        {
          NS_LOG_LOGIC ("Packet with UID " << it->first << " has latency times "
              << lt.m_startTime << " " << lt.m_endTime);
          TimeValue globalClock;
          NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", globalClock);
          calc->Update (NanoSeconds ((uint64_t)((lt.m_endTime.GetNanoSeconds () - lt.m_startTime.GetNanoSeconds ()) * 1.0
              / globalClock.Get ().GetNanoSeconds ())));
        }
    }
}

int
main (int argc, char *argv[])
{
  NS_ASSERT_MSG (numberOfNodes % hSize == 0,
      "The number of nodes ("<< numberOfNodes
      <<") must be a multiple of the number of nodes on the horizontal axis ("
      << hSize << ")");

  int globalClock = 1; // in seconds
  double injectionProbability (0.1);
  int dataPacketSpeedup (1);
  uint64_t bufferSize (9);
  std::string trafficPattern = "UniformRandom";

  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue<double> ("injection-probability", "The packet injection probability.", injectionProbability);
  cmd.AddValue<int> ("data-packet-speedup", "The speedup used for data packets (compared to head packets)", dataPacketSpeedup);
  cmd.AddValue<uint64_t> ("buffer-size", "The size of the input channel buffers (measured in multiples of packet size)", bufferSize);
  cmd.AddValue<std::string> ("traffic-pattern", "The traffic pattern "
      "(possible values: UniformRandom, BitMatrixTranspose, BitComplement, BitReverse, DestinationSpecified). "
      "The default one is UniformRandom and is used when nothing or an invalid value is specified.", trafficPattern);
  cmd.Parse (argc, argv);

  // set the global parameters
  NocRegistry::GetInstance ()->SetAttribute ("DataPacketSpeedup", IntegerValue (dataPacketSpeedup));
  NocRegistry::GetInstance ()->SetAttribute ("GlobalClock", TimeValue (Seconds (globalClock)));

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
      "MaxPackets", UintegerValue (bufferSize));

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
  routingProtocolFactory.Set ("RouteXFirst", BooleanValue (false));

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
  
    for (unsigned int i = 0; i < nodes.GetN(); ++i)
    {
      NocSyncApplicationHelper nocSyncAppHelper (nodes, devs, hSize);
      nocSyncAppHelper.SetAttribute ("NumberOfPackets", UintegerValue (9));
      nocSyncAppHelper.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
      nocSyncAppHelper.SetAttribute ("TrafficPattern", EnumValue (
          NocSyncApplication::TrafficPatternFromString (trafficPattern)));
      //  nocSyncAppHelper.SetAttribute ("Destination", UintegerValue (15)); // destination
//      nocSyncAppHelper.SetAttribute ("MaxPackets", UintegerValue (100));
      nocSyncAppHelper.SetAttribute ("WarmupCycles", UintegerValue (1000));
      ApplicationContainer apps = nocSyncAppHelper.Install (nodes.Get (i)); // source
      double startTime = 0.0;
      apps.Start (Seconds (startTime));
//      apps.Stop (Seconds (10.0));
      // the application can also be limited by MaxPackets (the two ways of ending the application are equivalent)
      long cycles = 10000;
      apps.Stop (Seconds (cycles * globalClock + startTime)); // stop = cycles*globalClock + start
    }
 
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the noc-sync-simulator.tr file
// Tracing should be kept disabled for big simulations
//  NS_LOG_INFO ("Configure Tracing.");
//    Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper> ("noc-sync-simulator.tr", std::ios_base::binary | std::ios_base::out);
//    noc->EnableAsciiAll (stream);

//  GtkConfigStore configstore;
//  configstore.ConfigureAttributes();

  //------------------------------------------------------------
  //-- Setup stats and data collection
  //--------------------------------------------

  std::string experiment ("noc-sync-simulator");

  std::string strategy ("noc-default");

  std::string context ("whole-network_10000-cycles_1000-warmup-cycles_message-length-9_xy-routing_wormhole-switching");

  // injection_probability-data_packet_speedup-traffic_pattern
  std::stringstream sstr;
  sstr << injectionProbability << "-" << dataPacketSpeedup << "-" << bufferSize << "-" << trafficPattern;
  std::string input (sstr.str());
  sstr.str ("");

  std::string runID;
  sstr << "run-" << input; // this is an ID. It must be unique
  runID = sstr.str ();

  // Create a DataCollector object to hold information about this run.
  DataCollector data;
  data.DescribeRun (experiment, strategy, input, runID);

  // Add any information we wish to record about this run.
  data.AddMetadata("author", "cipi");

  // This counter tracks how many complete messages are injected
  // (if there are messages for which only a part of them is injected into the network,
  // then such messages are not counted)
  Ptr<CounterCalculator<> > appMessageInjected = CreateObject<CounterCalculator<> > ();
  appMessageInjected->SetKey ("complete-messages-injected");
  appMessageInjected->SetContext (context);
  // we have to put $ because MessageInjected is not an Attribute
  // by using * we specify that we count for all the messages, injected by all applications in all nodes
  // see http://www.nsnam.org/wiki/index.php/HOWTO_determine_the_path_of_an_attribute_or_trace_source
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocSyncApplication/MessageInjected",
                  MakeBoundCallback (&MessageInjectedCallback, appMessageInjected));
  data.AddDataCalculator (appMessageInjected);



  // This counter tracks how many packets are injected
  Ptr<CounterCalculator<> > appPacketInjected = CreateObject<CounterCalculator<> > ();
  appPacketInjected->SetKey ("packets-injected");
  appPacketInjected->SetContext (context);
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocSyncApplication/Tx",
                  MakeBoundCallback (&PacketInjectedCallback, appPacketInjected));
  data.AddDataCalculator (appPacketInjected);



  Ptr<TimeMinMaxAvgTotalCalculator> latencyStat = CreateObject<TimeMinMaxAvgTotalCalculator>();
  latencyStat->SetKey ("latency");
  latencyStat->SetContext (context);
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocSyncApplication/PacketReceived",
                  MakeBoundCallback (&PacketReceivedCallback, 0));
  data.AddDataCalculator(latencyStat);



  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  // this must be done after the simulation ended
  ComputeLatenciesOfMessages (latencyStat);

  //------------------------------------------------------------
  //-- Generate statistics output.
  //--------------------------------------------

  // NOTE: Deleting the database here is useful only for a single test
  // If using a script, the script can do this
  // Actually, this must be commented out when using a script that runs multiple simulations
//  remove ("data.db");

  // Pick an output writer based in the requested format.
  Ptr<DataOutputInterface> output = 0;
  NS_LOG_INFO ("Creating sqlite formatted data output.");
  output = CreateObject<SqliteDataOutput> ();
  output->Output (data);

  return 0;
}
