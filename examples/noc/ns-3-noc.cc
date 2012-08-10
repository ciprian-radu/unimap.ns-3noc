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

// The ns-3 NoC simulator
// (currently works only with a 2D mesh topology)
// Notes:
//      1. We consider that the ns-3 Packet class models a flit (hence a small problem of semantics)

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
#include "ns3/uinteger.h"
#include "ns3/stats-module.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/nstime.h"
#include <map>
#include "ns3/output-stream-wrapper.h"
#include "ns3/sqlite-data-output.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ns-3NoC");

/**
 * Counts the packets injected into the network. A packet is made of flits.
 */
// parameters path and packet are required by the Trace Source Accesor
void PacketInjectedCallback (Ptr<CounterCalculator<> > calc, std::string path, Ptr<const Packet> flit)
{
  NS_LOG_LOGIC ("Packet injected counted in the " << calc->GetKey () << " statistic");

  calc->Update();
}

/**
 * Counts the flits injected into the network
 */
void FlitInjectedCallback (Ptr<CounterCalculator<> > calc, std::string path, Ptr<const Packet> flit)
{
  NS_LOG_LOGIC ("Flit injected counted in the " << calc->GetKey () << " statistic");

  calc->Update();
}

/**
 * Simple data structure for keeping the latency times of a packet
 */
class LatencyTimes
{
public:

  /** the time when the head of the packet is injected into the network */
  Time m_startTime;

  /** the time when the tail of the packet reaches the destination */
  Time m_endTime;
};

/** keeps the latency time of each fully received packet */
std::map<uint32_t, LatencyTimes> g_latencyOfPackets;

/**
 * Each time a flit is received at a node, this function gets invoked.
 *
 * \param dummy dummy parameter, not actually used (required by MakeBoundCall (...))
 * \param path the path with the receiving net device
 * \param flit the received flit
 */
void FlitReceivedCallback (int dummy, std::string path, Ptr<const Packet> flit)
{
  NS_LOG_LOGIC ("Flit with UID " << flit->GetUid () << " received");

  LatencyTimes latencyTimes;

  uint32_t uid;
  NocPacketTag tag;
  flit->PeekPacketTag (tag);
  NocPacket::Type flitType = tag.GetPacketType ();
  switch (flitType)
    {
  case NocPacket::HEAD:
    NS_LOG_LOGIC ("Flit type is head");
    latencyTimes = g_latencyOfPackets[flit->GetUid ()];
    latencyTimes.m_startTime = tag.GetInjectionTime ();
    break;
  case NocPacket::DATA:
    NS_LOG_LOGIC ("Flit type is data");
    break;
  case NocPacket::TAIL:
    NS_LOG_LOGIC ("Flit type is tail");
    uid = tag.GetPacketHeadUid ();
    latencyTimes = g_latencyOfPackets[uid];
    latencyTimes.m_endTime = tag.GetReceiveTime ();
    break;
  case NocPacket::UNKNOWN:
  default:
    NS_LOG_ERROR ("Flit type is unknown");
    break;
    }

  if (NocPacket::HEAD == flitType)
    {
      g_latencyOfPackets[flit->GetUid ()] = latencyTimes;
    }
  if (NocPacket::TAIL == flitType)
    {
      uid = tag.GetPacketHeadUid ();
      g_latencyOfPackets[uid] = latencyTimes;
    }

  NS_LOG_LOGIC ("Flit " << *flit << " has injection time " << tag.GetInjectionTime()
      << " and receive time " << tag.GetReceiveTime ());
  NS_ASSERT (tag.GetReceiveTime ().GetPicoSeconds () - tag.GetInjectionTime().GetPicoSeconds () >= 0);
}

/**
 * Computes the latency of each fully received packet. A packet is fully received when its
 * even its tail was received.
 *
 * The latency of a network is the time required for a packet to traverse the network,
 * from the time the head of the packet arrives at the input port to the time the tail of
 * the packet departs the output port.
 *
 * \param the latency calculator
 */
void ComputeLatenciesOfPackets (Ptr<TimeMinMaxAvgTotalCalculator > calc)
{
  NS_LOG_FUNCTION_NOARGS ();

  std::map<uint32_t, LatencyTimes>::iterator it;
  for (it = g_latencyOfPackets.begin (); it != g_latencyOfPackets.end (); it++)
    {
      LatencyTimes lt = it->second;
      if (lt.m_endTime.IsZero ())
        {
          NS_LOG_LOGIC ("The packet with the head with UID " << it->first << " has zero end time. "
              "This means the tail was not received and therefore, "
              "the latency of this packet is not considered.");
        }
      else
        {
          NS_LOG_LOGIC ("Packet with UID " << it->first << " has latency times "
              << lt.m_startTime << " " << lt.m_endTime);
          TimeValue globalClock;
          NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", globalClock);
          calc->Update ((lt.m_endTime - lt.m_startTime) / globalClock.Get ());
        }
    }
}

int
main (int argc, char *argv[])
{
  // ns-3 NoC parameters

  std::string experiment (""); // mandatory

  std::string strategy (""); // mandatory

  std::string input (""); // mandatory

  std::string description (""); // optional

  // a trial (simulation) is described by a run and a context (AKA name, in the database - SINGLETONS table)
  std::string run (""); // mandatory

  std::string author (""); // optional

  // the number of nodes from the NoC
  uint32_t numberOfNodes = 16;

  // how many nodes a 2D mesh has horizontally
  uint32_t hSize = 4;

  Time globalClock = PicoSeconds (1000); // 1 ns -> NoC @ 1GHz

  uint64_t flitSize = 32; // in bytes

  uint64_t flitsPerPacket = 9;

  double injectionProbability (1);

  int dataFlitSpeedup (1);

  uint64_t bufferSize (9);

  std::string trafficPattern = "UniformRandom";

  uint64_t warmupCycles = 1000;

  // the number of simulation cycles includes the warmup cycles
  uint64_t simulationCycles = 10000;

  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue<std::string> ("experiment", "The experiment is the study of which this trial (AKA simulation) is a member (mandatory parameter).", experiment);
  cmd.AddValue<std::string> ("strategy", "The strategy is the code or parameters being examined in this trial (mandatory parameter).", strategy);
  cmd.AddValue<std::string> ("input", "The input is the particular problem given to this trial (mandatory parameter).", input);
  cmd.AddValue<std::string> ("description", "The description can be used to specify the characteristics of the simulated NoC (optional parameter).", description);
  cmd.AddValue<std::string> ("run", "The run is a unique identifier for this trial with which it's information is tagged for identification in later analysis (mandatory parameter).", run);
  cmd.AddValue<std::string> ("author", "The author of this simulation (optional parameter).", author);
  cmd.AddValue<uint32_t> ("nodes", "The number of nodes from the NoC (default is 16)", numberOfNodes);
  cmd.AddValue<uint32_t> ("h-size", "How many nodes a 2D mesh has horizontally (default is 4)", hSize);
  cmd.AddValue<uint64_t> ("flit-size", "The size of a flit, in bytes (default is 32, minimum value is given by the size of the packet header)", flitSize);
  cmd.AddValue<uint64_t> ("flits-per-packet", "How many flits a packet has (default is 9, minimum value is 2)", flitsPerPacket);
  cmd.AddValue<double> ("injection-probability", "The packet injection probability (default is 1 (maximum), minimum is zero).", injectionProbability);
  cmd.AddValue<int> ("data-packet-speedup", "The speedup used for data packets (compared to head packets) (default is 1 (minimum value) - no speedup)", dataFlitSpeedup);
  cmd.AddValue<uint64_t> ("buffer-size", "The size of the input channel buffers (measured in multiples of packet size) (default is 9)", bufferSize);
  cmd.AddValue<std::string> ("traffic-pattern", "The traffic pattern "
      "(possible values: UniformRandom - default value, BitMatrixTranspose, BitComplement, BitReverse, DestinationSpecified). "
      "The default one is UniformRandom and is used when nothing or an invalid value is specified.", trafficPattern);
  cmd.AddValue<uint64_t> ("warmup-cycles", "The number of simulation warm-up cycles (default is 1000)", warmupCycles);
  cmd.AddValue<uint64_t> ("simulation-cycles", "The number of simulation cycles (includes the warm-up cycles, default is 10000)", simulationCycles);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("ns-3 NoC simulator");

  NS_ASSERT_MSG (numberOfNodes % hSize == 0,
      "The number of nodes ("<< numberOfNodes
      <<") must be a multiple of the number of nodes on the horizontal axis ("
      << hSize << ")");
  NocRegistry::GetInstance ()->SetAttribute ("FlitSize", IntegerValue (flitSize * 8));
  NS_ASSERT_MSG (flitsPerPacket >= 2, "At least 2 flits per packet are required!");
  NS_ASSERT_MSG (injectionProbability >= 0 && injectionProbability <= 1, "Injection probability must be in [0,1]!");
  NS_ASSERT_MSG (dataFlitSpeedup >= 1, "Data packet speedup must be >= 1!");
  // the buffer size is allowed to be any number >= 0
  NS_ASSERT_MSG (simulationCycles > warmupCycles, "The number of simulation cycles is not greater than the number of warm-up cycles!");

  // set the global parameters
  NocRegistry::GetInstance ()->SetAttribute ("DataPacketSpeedup", IntegerValue (dataFlitSpeedup));
  NocRegistry::GetInstance ()->SetAttribute ("GlobalClock", TimeValue (globalClock));

  // Here, we will explicitly create the NoC nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  for (unsigned int i = 0; i < numberOfNodes; ++i)
    {
      Ptr<NocNode> nocNode = CreateObject<NocNode> ();
      nodes.Add (nocNode);
    }
//  nodes.Create (numberOfNodes);

  NS_LOG_INFO ("Build Topology.");
  Ptr<NocTopology> noc = CreateObject<NocTorus2D> ();
//  Ptr<NocTopology> noc = CreateObject<NocIrvineMesh2D> ();
  noc->SetAttribute ("hSize", UintegerValue (hSize));
  int64_t dimensions = 2;
  vector<Ptr<NocValue> > size(dimensions);
  NocRegistry::GetInstance ()->SetAttribute ("NoCDimensions", IntegerValue (dimensions));
  size.at (0) = CreateObject<NocValue> (hSize);
  size.at (1) = CreateObject<NocValue> (numberOfNodes / hSize);

  // set channel bandwidth to 1 flit / network clock
  // the channel's bandwidth is obviously expressed in bits / s
  // however, in order to avoid losing precision, we work with PicoSeconds (instead of Seconds)
  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate ((uint64_t) (1e12 * (flitSize * 8)
      / globalClock.GetPicoSeconds ()))));
  // the channel has no propagation delay
  noc->SetChannelAttribute ("Delay", TimeValue (PicoSeconds (0)));

// By default, we use full-duplex communication
  //  noc->SetChannelAttribute ("FullDuplex", BooleanValue (false));

// Setting the size of the input buffers, uniformly
  noc->SetInQueue ("ns3::DropTailQueue",
      "Mode", EnumValue (DropTailQueue::PACKETS),
      "MaxPackets", UintegerValue (bufferSize));

  // configure the routers
  noc->SetRouter ("ns3::FourWayRouter");
//  noc->SetRouter ("ns3::IrvineRouter");
//  noc->SetRouter ("ns3::IrvineLoadRouter");

  // WARNING setting properties for objects in this manner means that all the created objects
  // will refer to the *same* object
  //
  // example: all routers will use the *same* load component; this is obviously incorrect
  // we therefore can't do something like this:
  //
  // Ptr<LoadRouterComponent> loadComponent = CreateObject<SlbLoadRouterComponent> ();

//  noc->SetRouterAttribute ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SlbLoadRouterComponent")));
//  noc->SetRouterAttribute ("LoadComponent", TypeIdValue (TypeId::LookupByName ("ns3::SoLoadRouterComponent")));

  // Do not forget about changing the routing protocol when changing the load router component

  // setting the routing protocol
//  noc->SetRoutingProtocol ("ns3::XyRouting");
//  noc->SetRoutingProtocolAttribute ("RouteXFirst", BooleanValue (false));

//  noc->SetRoutingProtocol ("ns3::SlbRouting");
//  noc->SetRoutingProtocolAttribute ("LoadThreshold", IntegerValue (30));

//  noc->SetRoutingProtocol ("ns3::SoRouting");

  noc->SetRoutingProtocol ("ns3::DorRouting");

  // setting the switching mechanism
//  noc->SetSwitchingProtocol ("ns3::SafSwitching");
//  noc->SetSwitchingProtocol ("ns3::VctSwitching");
  noc->SetSwitchingProtocol ("ns3::WormholeSwitching");

  // installing the topology
  NetDeviceContainer devs = noc->Install (nodes);
  NocRegistry::GetInstance ()->SetAttribute ("NoCTopology", PointerValue (noc));
  // done with installing the topology

  NS_LOG_INFO ("Create Applications.");
  
    for (unsigned int i = 0; i < nodes.GetN(); ++i)
    {
      NocSyncApplicationHelper nocSyncAppHelper (nodes, devs, size);
      nocSyncAppHelper.SetAttribute ("NumberOfFlits", UintegerValue (flitsPerPacket));
      nocSyncAppHelper.SetAttribute ("InjectionProbability", DoubleValue (injectionProbability));
      nocSyncAppHelper.SetAttribute ("TrafficPattern", EnumValue (
          NocSyncApplication::TrafficPatternFromString (trafficPattern)));
      //  nocSyncAppHelper.SetAttribute ("Destination", UintegerValue (15)); // destination
//      nocSyncAppHelper.SetAttribute ("MaxPackets", UintegerValue (100));
      nocSyncAppHelper.SetAttribute ("WarmupCycles", UintegerValue (warmupCycles));
      ApplicationContainer apps = nocSyncAppHelper.Install (nodes.Get (i)); // source
      uint64_t startTime = 0;
      apps.Start (PicoSeconds (startTime));
//      apps.Stop (PicoSeconds (10.0));
      // the application can also be limited by MaxPackets (the two ways of ending the application are equivalent)
      apps.Stop (PicoSeconds (simulationCycles * globalClock.GetPicoSeconds() + startTime)); // stop = simulationCycles * globalClock + start
    }
 
// Configure tracing of all enqueue, dequeue, and NetDevice receive events
// Trace output will be sent to the ns-3NoC.tr file
// Tracing should be kept disabled for big simulations
//  NS_LOG_INFO ("Configure Tracing.");
//    Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper> ("ns-3NoC.tr", std::ios_base::binary | std::ios_base::out);
//    noc->EnableAsciiAll (stream);

//  GtkConfigStore configstore;
//  configstore.ConfigureAttributes();

  // Setup statistics and data collection

  // a trial (simulation) is described by a run and a context (AKA name, in the database - SINGLETONS table)

  std::string context ("average packet latency");

  NS_ASSERT_MSG (experiment.size() > 0, "Please name the experiment you are performing with ns-3 NoC");
  NS_ASSERT_MSG (strategy.size() > 0, "Please name the strategy used in this experiment (CSV values)");
  NS_ASSERT_MSG (input.size() > 0, "Please specify the input of this simulation (CSV values)");
  // description is optional
  NS_ASSERT_MSG (run.size() > 0, "Please specify the unique identifier of this simulation (CSV values)");
  // author is optional

  // Create a DataCollector object to hold information about this run.
  DataCollector data;
  data.DescribeRun (experiment, strategy, input, run, description);

  // Add any information we wish to record about this run.
  if (author.size() > 0) {
      data.AddMetadata("author", author);
  }

  // This counter tracks how many complete packets are injected
  // (if there are packets for which only a part of them is injected into the network,
  // then such messages are not counted)
  Ptr<CounterCalculator<> > appPacketInjected = CreateObject<CounterCalculator<> > ();
  appPacketInjected->SetKey ("packets-injected");
  appPacketInjected->SetContext (context);
  // we have to put $ because MessageInjected is not an Attribute
  // by using * we specify that we count for all the messages, injected by all applications in all nodes
  // see http://www.nsnam.org/wiki/index.php/HOWTO_determine_the_path_of_an_attribute_or_trace_source
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocSyncApplication/PacketInjected",
                  MakeBoundCallback (&PacketInjectedCallback, appPacketInjected));
  data.AddDataCalculator (appPacketInjected);

  // This counter tracks how many flits are injected
  Ptr<CounterCalculator<> > appFlitInjected = CreateObject<CounterCalculator<> > ();
  appFlitInjected->SetKey ("flits-injected");
  appFlitInjected->SetContext (context);
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocSyncApplication/FlitInjected",
                  MakeBoundCallback (&FlitInjectedCallback, appFlitInjected));
  data.AddDataCalculator (appFlitInjected);

  // Computes the average packet latency
  Ptr<TimeMinMaxAvgTotalCalculator> latencyStat = CreateObject<TimeMinMaxAvgTotalCalculator>();
  latencyStat->SetKey ("latency");
  latencyStat->SetContext (context);
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocSyncApplication/FlitReceived",
                  MakeBoundCallback (&FlitReceivedCallback, 0));
  data.AddDataCalculator(latencyStat);

  // start the simulation

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  NS_LOG_INFO ("Done.");
  NS_LOG_INFO ("Simulation time is " << Simulator::Now ());
  Simulator::Destroy ();

  // this must be done after the simulation ended
  ComputeLatenciesOfPackets (latencyStat);

  // Generate statistics output

// NOTE: Deleting the database here is useful only for a single test
// This must obviously be commented out when using a script that runs multiple simulations
//  remove ("data.db");

  // Pick an output writer based in the requested format.
  NS_LOG_INFO ("Creating sqlite formatted data output.");
  Ptr<DataOutputInterface> output = CreateObject<SqliteDataOutput> ();
  output->Output (data);

  return 0;
}
