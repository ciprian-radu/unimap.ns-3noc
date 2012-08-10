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
#include <iomanip>

#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/node-module.h"
#include "ns3/topology-module.h"
#include "ns3/noc-ctg-application.h"
#include "ns3/noc-ctg-application-helper.h"
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
#include <list>
#include "ns3/output-stream-wrapper.h"
#include <sys/time.h>
#include "ns3/sqlite-data-output.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("ns-3NoCUniMap");

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

  bool justSaveTopology = false; // optional

  std::string experiment (""); // mandatory

  std::string strategy (""); // mandatory

  std::string input (""); // mandatory

  std::string description (""); // optional

  // a trial (simulation) is described by a run and a context (AKA name, in the database - SINGLETONS table)
  std::string run (""); // mandatory

  std::string author (""); // optional

  // the path to the XML file containing the mapping
  string mappingXmlFilePath;

  // the number of nodes from the NoC
  int numberOfNodes = 16;

  // how many nodes a 2D mesh has horizontally
  int hSize = 4;

  uint64_t nocFrequency = 1000000000; // NoC @ 1GHz

  uint64_t flitSize = 32; // in bytes

  uint64_t flitsPerPacket = 9;

//  double injectionProbability (1);

  int dataFlitSpeedup (1);

  uint64_t channelBandwidth;

  uint64_t channelDelay = 0;

  double channelLength = 50;

  uint64_t bufferSize (9);

  bool routeXFirst = true; // optional

  uint64_t warmupCycles = 1000;

  // the number of simulation cycles includes the warmup cycles
  uint64_t simulationCycles = 10000;

  uint64_t ctgIterations = 1;

  std::string outputFilePath ("./ns-3-noc-output.txt"); // optional

  bool redirectStdout = false; // optional

  bool redirectStderr = false; // optional

  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue<bool> ("just-save-topology", "If set to true, this application doesn't do any simulation, it just saves the NoC topology (optional parameter, false by default).", justSaveTopology);
  cmd.AddValue<string> ("experiment", "The experiment is the study of which this trial (AKA simulation) is a member (mandatory parameter when just-save-topology = false).", experiment);
  cmd.AddValue<string> ("strategy", "The strategy is the code or parameters being examined in this trial (mandatory paramete rwhen just-save-topology = false).", strategy);
  cmd.AddValue<string> ("input", "The input is the particular problem given to this trial (mandatory parameter when just-save-topology = false).", input);
  cmd.AddValue<string> ("description", "The description can be used to specify the characteristics of the simulated NoC (optional parameter).", description);
  cmd.AddValue<string> ("run", "The run is a unique identifier for this trial with which it's information is tagged for identification in later analysis (mandatory parameter when just-save-topology = false).", run);
  cmd.AddValue<string> ("author", "The author of this simulation (optional parameter).", author);
  cmd.AddValue<string> ("mapping-file-path", "The path to the XML file that contains the application mapping (mandatory parameter when just-save-topology = false)", mappingXmlFilePath);
  cmd.AddValue<uint64_t> ("frequency", "The NoC operating frequency, expressed in Hertz (default is 1000000000 Hz, i.e. 1 GHz)", nocFrequency);
  cmd.AddValue<int> ("nodes", "The number of nodes from the NoC (default is 16)", numberOfNodes);
  cmd.AddValue<int> ("h-size", "How many nodes a 2D mesh has horizontally (default is 4)", hSize);
  cmd.AddValue<uint64_t> ("flit-size", "The size of a flit, in bytes (default is 32, minimum value is given by the size of the packet header)", flitSize);
  cmd.AddValue<uint64_t> ("flits-per-packet", "How many flits a packet has (default is 9, minimum value is 2)", flitsPerPacket);
//  cmd.AddValue<double> ("injection-probability", "The packet injection probability (default is 1 (maximum), minimum is zero).", injectionProbability);
  cmd.AddValue<int> ("data-flit-speedup", "The speedup used for data flits (compared to head flits) (default is 1 (minimum value) - no speedup)", dataFlitSpeedup);
  cmd.AddValue<uint64_t> ("channel-bandwidth", "The NoC channels bandwidth, in bits per second (default is the value that allows 1 flit per NoC clock cycle)", channelBandwidth);
  cmd.AddValue<uint64_t> ("channel-delay", "The NoC channels propagation delay, in picoseconds (default is zero)", channelDelay);
  cmd.AddValue<double> ("channel-length", "The NoC channels length, in micrometers (default is 50)", channelLength);
  cmd.AddValue<uint64_t> ("buffer-size", "The size of the input channel buffers (measured in flits) (default is 9)", bufferSize);
  cmd.AddValue<bool> ("route-X-first", "If set to false, YX routing is used, not XY (optional parameter, true by default).", routeXFirst);
  cmd.AddValue<uint64_t> ("warmup-cycles", "The number of simulation warm-up cycles (default is 1000)", warmupCycles);
  cmd.AddValue<uint64_t> ("simulation-cycles", "The number of simulation cycles (includes the warm-up cycles, default is 10000)", simulationCycles);
  cmd.AddValue<uint64_t> ("ctg-iterations", "How many times a Communication Task Graph has to be iterated (default value is 1, i.e. the CTG is not reiterated)", ctgIterations);
  cmd.AddValue<string> ("output-file", "The path to a file where the simulator will put its output. The path must also contain the name of the output file. Default is: ./ns-3-noc-output.txt", outputFilePath);
  cmd.AddValue<bool> ("redirect-stdout", "If set to true, stdout is redirected to a file named like the output file but with .out.log extension (optional parameter, false by default).", redirectStdout);
  cmd.AddValue<bool> ("redirect-stderr", "If set to true, stderr is redirected to a file named like the output file but with .out.log extension (optional parameter, false by default).", redirectStderr);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("ns-3 NoC simulator for UniMap ( https://code.google.com/p/unimap/ )");

  if (redirectStdout)
    {
      stringstream ssOut;
      ssOut << outputFilePath << ".out.log";
      NS_LOG_INFO ("Redirecting stdout to " << ssOut.str ());
      freopen (ssOut.str ().c_str (), "w", stdout);
    }

  if (redirectStderr)
    {
      stringstream ssErr;
      ssErr << outputFilePath << ".err.log";
      NS_LOG_INFO ("Redirecting stderr to " << ssErr.str ());
      freopen (ssErr.str ().c_str (), "w", stderr);
      NS_LOG_INFO ("ns-3 NoC simulator for UniMap ( https://code.google.com/p/unimap/ )");
    }

  cout << "The following command line was used to launch ns-3 NoC:" << endl;
  for (int i = 0; i < argc; i++)
    {
      cout << argv[i] << " ";
    }
  cout << endl;

  Time globalClock = PicoSeconds ((uint64_t) (1e12 * 1.0 / nocFrequency)); // 1 ns -> NoC @ 1GHz
  NS_LOG_INFO ("NoC clock cycle = " << globalClock.GetPicoSeconds ());
  channelBandwidth = (uint64_t) (1e12 * (flitSize * 8) / globalClock.GetPicoSeconds ());

  if (!justSaveTopology) {
      NS_ASSERT_MSG (mappingXmlFilePath.size() > 0, "Please specify the file path for the XML containing the mapping!");
  }
  NS_ASSERT_MSG (numberOfNodes % hSize == 0,
      "The number of nodes ("<< numberOfNodes
      <<") must be a multiple of the number of nodes on the horizontal axis ("
      << hSize << ")");
  if (!justSaveTopology) {
    NocRegistry::GetInstance ()->SetAttribute ("FlitSize", IntegerValue (flitSize * 8));
    NS_ASSERT_MSG (flitsPerPacket >= 2, "At least 2 flits per packet are required!");
//  NS_ASSERT_MSG (injectionProbability >= 0 && injectionProbability <= 1, "Injection probability must be in [0,1]!");
    NS_ASSERT_MSG (dataFlitSpeedup >= 1, "Data packet speedup must be >= 1!");
    // the buffer size is allowed to be any number >= 0
    NS_ASSERT_MSG (simulationCycles > warmupCycles, "The number of simulation cycles is not greater than the number of warm-up cycles!");
  }

  // set the global parameters
  NocRegistry::GetInstance ()->SetAttribute ("DataPacketSpeedup", IntegerValue (dataFlitSpeedup));
  NocRegistry::GetInstance ()->SetAttribute ("GlobalClock", TimeValue (globalClock));

  // Here, we will explicitly create the NoC nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  for (int i = 0; i < numberOfNodes; ++i)
    {
      Ptr<NocNode> nocNode = CreateObject<NocNode> ();
      nodes.Add (nocNode);
    }
//  nodes.Create (numberOfNodes);

  NS_LOG_INFO ("Build Topology.");
  int64_t dimensions = 2;
//  vector<Ptr<NocValue> > size(dimensions);
  NocRegistry::GetInstance ()->SetAttribute ("NoCDimensions", IntegerValue (dimensions));
//  size.at (0) = CreateObject<NocValue> (hSize);
//  size.at (1) = CreateObject<NocValue> (numberOfNodes / hSize);
//  Ptr<NocTopology> noc = CreateObject<NocMeshND> (size);
  // FIXME use NocMeshND (requires changing NoC CTG application)
  Ptr<NocTopology> noc = CreateObject<NocMesh2D> ();
  noc->SetAttribute ("hSize", UintegerValue (hSize));

  // set channel bandwidth to 1 flit / network clock
  // the channel's bandwidth is obviously expressed in bits / s
  // however, in order to avoid losing precision, we work with PicoSeconds (instead of Seconds)
  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate (channelBandwidth)));
  // the channel has no propagation delay
  noc->SetChannelAttribute ("Delay", TimeValue (PicoSeconds (channelDelay)));
  // the channel length, in micrometers
  noc->SetChannelAttribute ("Length", DoubleValue (channelLength));

// By default, we use full-duplex communication
  //  noc->SetChannelAttribute ("FullDuplex", BooleanValue (false));

// Setting the size of the input buffers, uniformly
  noc->SetInQueue ("ns3::DropTailQueue",
      "Mode", EnumValue (DropTailQueue::PACKETS),
      "MaxPackets", UintegerValue (bufferSize));

  // configure the routers
  string routerClass;
  routerClass = "ns3::FourWayRouter";
//  routerClass = "ns3::IrvineLoadRouter";
  noc->SetRouter (routerClass);

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
  string routingProtocolClass;
  routingProtocolClass = "ns3::XyRouting";
//  routingProtocolClass = "ns3::SlbRouting";
//  routingProtocolClass = "ns3::SoRouting";
//  routingProtocolClass = "ns3::DorRouting";
  noc->SetRoutingProtocol (routingProtocolClass);
  noc->SetRoutingProtocolAttribute ("RouteXFirst", BooleanValue (routeXFirst));

//  noc->SetRoutingProtocolAttribute ("LoadThreshold", IntegerValue (30));

  // setting the switching mechanism
  string switchingProtocolClass;
  switchingProtocolClass = "ns3::WormholeSwitching";
//  switchingProtocolClass = "ns3::VctSwitching";
//  switchingProtocolClass = "ns3::SafSwitching";
  noc->SetSwitchingProtocol (switchingProtocolClass);

  // installing the topology
  NetDeviceContainer devs = noc->Install (nodes);
  NocRegistry::GetInstance ()->SetAttribute ("NoCTopology", PointerValue (noc));
  // done with installing the topology

  NS_LOG_INFO ("Saving the NoC topology in XML format.");
  noc->SaveTopology (nodes, "../NoC-XML/src/ro/ulbsibiu/acaps/noc/topology/mesh2D");

  if (!justSaveTopology)
    {
      NS_LOG_INFO ("Create CTG based Applications.");
      NocCtgApplicationHelper ctgApplicationHelper (mappingXmlFilePath, ctgIterations, // the number of CTG iterations
          flitsPerPacket, simulationCycles, nodes, devs, hSize);
      ctgApplicationHelper.Initialize ();

    // Configure tracing of all enqueue, dequeue, and NetDevice receive events
    // Trace output will be sent to the ns-3NoCUniMap.tr file
  // Tracing should be kept disabled for big simulations
    NS_LOG_INFO ("Configure Tracing.");
    Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper> ("ns-3NoCUniMap.tr", std::ios_base::binary | std::ios_base::out);
    noc->EnableAsciiAll (stream);

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
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocCtgApplication/PacketInjected",
                    MakeBoundCallback (&PacketInjectedCallback, appPacketInjected));
    data.AddDataCalculator (appPacketInjected);

    // This counter tracks how many flits are injected
    Ptr<CounterCalculator<> > appFlitInjected = CreateObject<CounterCalculator<> > ();
    appFlitInjected->SetKey ("flits-injected");
    appFlitInjected->SetContext (context);
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocCtgApplication/FlitInjected",
                    MakeBoundCallback (&FlitInjectedCallback, appFlitInjected));
    data.AddDataCalculator (appFlitInjected);

    // Computes the average packet latency
    Ptr<TimeMinMaxAvgTotalCalculator> latencyStat = CreateObject<TimeMinMaxAvgTotalCalculator>();
    latencyStat->SetKey ("latency");
    latencyStat->SetContext (context);
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::NocCtgApplication/FlitReceived",
                    MakeBoundCallback (&FlitReceivedCallback, 0));
    data.AddDataCalculator(latencyStat);

    // start the simulation

    NS_LOG_INFO ("Run Simulation.");
    timespec startTime;
    clock_gettime(CLOCK_REALTIME, &startTime);
    Simulator::Run ();
    timespec endTime;
    clock_gettime(CLOCK_REALTIME, &endTime);
    NS_LOG_INFO ("Done.");

    Time simulatedTime = Simulator:: Now ();
    double applicationRuntime = simulatedTime.GetPicoSeconds () * 1.0 / 1e12; // application runtime, in seconds
    NS_LOG_INFO ("Simulated time is " << simulatedTime);
    double nocDynamicPower = noc->GetDynamicPower ();
    NS_LOG_INFO ("NoC dynamic power: " << nocDynamicPower << " W");
    double nocLeakagePower = noc->GetLeakagePower ();
    NS_LOG_INFO ("NoC leakage power: " << nocLeakagePower << " W");
    double nocTotalPower = noc->GetTotalPower ();
    NS_LOG_INFO ("NoC total power: " << nocTotalPower << " W");
    double nocArea = noc->GetArea ();
    NS_LOG_INFO ("NoC area: " << nocArea << " um^2");
    Simulator::Destroy ();

    double coresArea = 0; // the area occupied by the cores, in m^2
    double coresEnergy = 0; // the energy consumed by the cores, in Joule

    list<NocCtgApplicationHelper::CoreData> coreDataList = ctgApplicationHelper.GetCoreDataList ();
    for (list<NocCtgApplicationHelper::CoreData>::iterator it = coreDataList.begin(); it != coreDataList.end(); it++)
      {
        NocCtgApplicationHelper::CoreData coreData = *it;
        NS_LOG_DEBUG ("Core with UID " << coreData.m_uid << ", ID " << coreData.m_id
            << ", APCG " << coreData.m_apcgId << ", has area " << coreData.m_area << " m^2");
        coresArea += coreData.m_area;

        double coreTotalExecTime = 0;
        for (map<string, double>::iterator it = coreData.m_power.begin(); it != coreData.m_power.end(); it++)
          {
            string taskId = it->first;
            double power = it->second;
            double execTime = ctgIterations * coreData.m_execTime[taskId];
            NS_LOG_DEBUG ("This core consumed " << power << " Watts " << " for " << execTime << " seconds, to execute task with ID " << taskId);
            coresEnergy += power * execTime;
            coreTotalExecTime += execTime;
          }
        NS_ASSERT_MSG (coreTotalExecTime <= applicationRuntime,
        		"Core total execution time (" << coreTotalExecTime << ") is higher than application runtime (" << applicationRuntime << ")!");
        NS_LOG_DEBUG ("This core was idle for " << applicationRuntime - coreTotalExecTime << " seconds (idle power is " << coreData.m_idlePower << ")");
        coresEnergy += coreData.m_idlePower * (applicationRuntime - coreTotalExecTime);
      }

    NS_LOG_INFO ("Cores area: " << coresArea << " mm^2");
    NS_LOG_INFO ("Cores energy: " << coresEnergy << " Joule");

    // this must be done after the simulation ended
    ComputeLatenciesOfPackets (latencyStat);

    // Write the simulation results in the output file
    ofstream outputFile (outputFilePath.c_str ());

    time_t rawtime;
    time (&rawtime);
    outputFile << setprecision (20);
    outputFile << "# Simulation results produced with the ns-3 Network-on-Chip (ns-3 NoC) simulator, on " << ctime (&rawtime);
    outputFile << "# ns-3 NoC is part of UniMap framework ( https://code.google.com/p/unimap/ )" << endl;
    outputFile << "# UniMap is part of the PhD work of Ciprian Radu ( http://webspace.ulbsibiu.ro/ciprian.radu/ ) and is available under GNU GPL v3 license" << endl;
    outputFile << endl;
    outputFile << "####################" << endl;
    outputFile << "# Input parameters #" << endl;
    outputFile << "####################" << endl;
    outputFile << endl;
    outputFile << "# NoC operating frequency, in Hertz" << endl;
    outputFile << "# noc-frequency = " << nocFrequency << endl;
    outputFile << endl;
    outputFile << "# NoC topology" << endl;
    outputFile << "# noc-topology = " << noc->GetInstanceTypeId () << endl;
    outputFile << endl;
    // FIXME add topology size after working with NocMeshND
    outputFile << "# NoC nodes" << endl;
    outputFile << "# noc-nodes = " << numberOfNodes << endl;
    outputFile << endl;
    outputFile << "# NoC horizontal size" << endl;
    outputFile << "# noc-h-size = " << hSize << endl;
    outputFile << endl;
    outputFile << "# Router" << endl;
    outputFile << "# " << routerClass << endl;
    outputFile << endl;
    outputFile << "# Routing protocol" << endl;
    outputFile << "# " << routingProtocolClass << endl;
    outputFile << endl;
    outputFile << "# Switching protocol" << endl;
    outputFile << "# " << switchingProtocolClass << endl;
    outputFile << endl;
    outputFile << "# Flit size, in bytes" << endl;
    outputFile << "# noc-flit-size = " << flitSize << endl;
    outputFile << endl;
    outputFile << "# Packet size, in flits" << endl;
    outputFile << "# noc-flits-per-packet = " << flitsPerPacket << endl;
    outputFile << endl;
    outputFile << "# Data flit speedup (how many times a data flit is sent faster than a header flit)" << endl;
    outputFile << "# noc-data-flit-speedup = " << dataFlitSpeedup << endl;
    outputFile << endl;
    outputFile << "# The size of each NoC buffer, in flits" << endl;
    outputFile << "# noc-buffer-size = " << bufferSize << endl;
    outputFile << endl;
    outputFile << "# NoC channel bandwidth, in bits per second" << endl;
    outputFile << "# noc-channel-bandwidth = " << channelBandwidth << endl;
    outputFile << endl;
    outputFile << "# NoC channel propagation delay, in picoseconds" << endl;
    outputFile << "# noc-channel-delay = " << channelDelay << endl;
    outputFile << endl;
    outputFile << "# NoC channel length, in micrometers" << endl;
    outputFile << "# noc-channel-length = " << channelLength << endl;
    outputFile << endl;
//    outputFile << "# The number of warmup cycles" << endl;
//    outputFile << "# warmup-cycles  = " << warmupCycles << endl;
//    outputFile << endl;
//    outputFile << "# The number of simulation cycles" << endl;
//    outputFile << "# simulation-cycles  = " << simulationCycles << endl;
//    outputFile << endl;
    outputFile << "# The number of CTG iterations" << endl;
    outputFile << "# ctg-iterations = " << ctgIterations << endl;
    outputFile << endl;
    outputFile << "# Mapping file path" << endl;
    outputFile << "# mapping-file-path = " << mappingXmlFilePath << endl;
    outputFile << endl;
    outputFile << "#####################" << endl;
    outputFile << "# Output parameters #" << endl;
    outputFile << "#####################" << endl;
    outputFile << endl;
    outputFile << "# Application runtime represents the amount of simulated time. It is expressed in seconds." << endl;
    outputFile << "application-runtime = " << applicationRuntime << endl;
    outputFile << endl;
    outputFile << "# The dynamic power consumed by the entire Network-on-Chip architecture. It is expressed in Watts." << endl;
    outputFile << "noc-dynamic-power = " << nocDynamicPower << endl;
    outputFile << endl;
    outputFile << "# The leakage power consumed by the entire Network-on-Chip architecture. It is expressed in Watts." << endl;
    outputFile << "noc-leakage-power = " << nocLeakagePower << endl;
    outputFile << endl;
    outputFile << "# The total power consumed by the entire Network-on-Chip architecture (dynamic power + leakage power). It is expressed in Watts." << endl;
    outputFile << "noc-power = " << nocTotalPower << endl;
    outputFile << endl;
    outputFile << "# The area occupied by the Network-on-Chip architecture. It is expressed in mm^2 (square millimeters)." << endl;
    outputFile << "noc-area = " << (nocArea / 1e6) << endl;
    outputFile << endl;
    outputFile << "# The energy consumed by the entire Network-on-Chip architecture. It is expressed in Joule and it is the product between total energy and application runtime." << endl;
    outputFile << "noc-energy = " << nocTotalPower * applicationRuntime << endl;
    outputFile << endl;
    outputFile << "# The area occupied by the IP cores. It is expressed in mm^2 (square millimeters)." << endl;
    outputFile << "cores-area = " << (coresArea * 1e6) << endl;
    outputFile << endl;
    outputFile << "# The energy consumed by the IP cores. It is expressed in Joule." << endl;
    outputFile << "cores-energy = " << coresEnergy << endl;
    outputFile << endl;
    outputFile << "# The area occupied by the entire System on Chip (SoC). It is expressed in mm^2 (square millimeters) and it sums noc-area and cores-area." << endl;
    outputFile << "soc-area = " << (nocArea / 1e6 + coresArea * 1e6) << endl;
    outputFile << endl;
    outputFile << "# The energy consumed by the entire System on Chip (SoC). It is expressed in Joule and it sums noc-energy and cores-energy." << endl;
    outputFile << "soc-energy = " << (nocTotalPower * applicationRuntime + coresEnergy) << endl;
    outputFile << endl;
    outputFile << "# How much time the simulation took. Is is expressed in seconds." << endl;
    outputFile << "simulation-runtime = " << endTime.tv_sec - startTime.tv_sec << endl;
    outputFile << endl;

    outputFile.close ();

    // Generate statistics output

  // NOTE: Deleting the database here is useful only for a single test
  // This must obviously be commented out when using a script that runs multiple simulations
  //  remove ("data.db");

    // Pick an output writer based in the requested format.
    NS_LOG_INFO ("Creating sqlite formatted data output.");
    Ptr<DataOutputInterface> output = CreateObject<SqliteDataOutput> (outputFilePath);
    output->Output (data);
    NS_LOG_INFO ("Finished.");
  }
  return 0;
}
