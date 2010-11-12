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
#include "ns3/helper-module.h"
#include "ns3/noc-ctg-application.h"
#include "ns3/noc-ctg-application-helper.h"
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

#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/mapping/mapping.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/apcg/apcg.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/ctg/ctg.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/core/core.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/task/task.hxx"

using namespace ns3;

using namespace research::noc::application_mapping::unified_framework::schema::mapping;
using namespace research::noc::application_mapping::unified_framework::schema::apcg;
using namespace research::noc::application_mapping::unified_framework::schema::ctg;
using namespace research::noc::application_mapping::unified_framework::schema::core;
using namespace research::noc::application_mapping::unified_framework::schema::task;

using xml_schema::flags;

const char FILE_SEPARATOR = '/';

NS_LOG_COMPONENT_DEFINE ("ns-3NoCUniMap");

/**
 * Counts the packets injected into the network. A packet is made of flits.
 */
// parameters path and packet are required by the Trace Source Accesor
void PacketInjectedCallback (Ptr<CounterCalculator<> > calc, std::string path, Ptr<const Packet> flit)
{
  NS_LOG_INFO ("Packet injected counted in the " << calc->GetKey () << " statistic");

  calc->Update();
}

/**
 * Counts the flits injected into the network
 */
void FlitInjectedCallback (Ptr<CounterCalculator<> > calc, std::string path, Ptr<const Packet> flit)
{
  NS_LOG_INFO ("Flit injected counted in the " << calc->GetKey () << " statistic");

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
  NS_LOG_INFO ("Packet with UID " << flit->GetUid () << " received");

  LatencyTimes latencyTimes;

  NocHeader header;
  flit->PeekHeader (header);

  uint32_t uid;
  NocPacketTag tag;
  flit->PeekPacketTag (tag);
  NocPacket::Type flitType = tag.GetPacketType ();
  switch (flitType)
    {
  case NocPacket::HEAD:
    NS_LOG_DEBUG ("Flit type is head");
    latencyTimes = g_latencyOfPackets[flit->GetUid ()];
    latencyTimes.m_startTime = tag.GetInjectionTime ();
    break;
  case NocPacket::DATA:
    NS_LOG_DEBUG ("Flit type is data");
    break;
  case NocPacket::TAIL:
    NS_LOG_DEBUG ("Flit type is tail");
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
  NS_ASSERT (tag.GetReceiveTime ().GetNanoSeconds () - tag.GetInjectionTime().GetNanoSeconds () >= 0);
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
          calc->Update (NanoSeconds ((uint64_t) ((lt.m_endTime.GetNanoSeconds () - lt.m_startTime.GetNanoSeconds ()) * 1.0
              / globalClock.Get ().GetNanoSeconds ())));
        }
    }
}

apcgType::core_type
findCoreInApcg(apcgType theApcgType, string coreId)
{
  apcgType::core_type theApcgCoreType ("");
  for (apcgType::core_iterator i (theApcgType.core ().begin ()); i != theApcgType.core ().end (); i++)
    {
      if ((*i).id () == coreId)
        {
          theApcgCoreType = *i;
          break;
        }
    }
  return theApcgCoreType;
}

string
findNodeIdForTask(apcgType theApcgType, mappingType theMappingType, string taskId)
{
  NS_LOG_FUNCTION_NOARGS ();

  string nodeId = "";

  for (apcgType::core_iterator i (theApcgType.core ().begin ()); i != theApcgType.core ().end (); i++)
    {
      apcgType::core_type theCoreType = *i;
      for (apcgType::core_type::task_const_iterator j (i->task ().begin ()); j != i->task ().end (); j++)
        {
          apcgType::core_type::task_type theApcgTaskType = *j;
          if (taskId == theApcgTaskType.id ())
            {
              for (mappingType::map_const_iterator k (theMappingType.map ().begin ()); k != theMappingType.map ().end (); k++)
                {
                  mapType theMapType = *k;

                  if (theMapType.core ().get () == theCoreType.id ())
                    {
                      nodeId = theMapType.node ();
                      goto done;
                    }
                }
            }
        }
    }

  done:

  if ("" == nodeId)
    {
      NS_LOG_ERROR ("Couldn't find the node of task with ID " << taskId);
    }
  else
    {
      NS_LOG_DEBUG ("Task with ID " << taskId << " belongs to node with ID " << nodeId);
    }

  return nodeId;
}

template<class T>
bool
fromString(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
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

  // the path to the XML file containing the mapping
  string mappingXmlFilePath;

  // the number of nodes from the NoC
  uint32_t numberOfNodes = 16;

  // how many nodes a 2D mesh has horizontally
  uint32_t hSize = 4;

  // Note: if you want to change the global clock from being measured in seconds,
  // be aware that the change must be done everywhere the global clock is used (NocRegistry references)
  Time globalClock = PicoSeconds (1000); // in picoseconds

  uint64_t flitSize = 32; // in bytes

  uint64_t flitsPerPacket = 9;

//  double injectionProbability (1);

  int dataFlitSpeedup (1);

  uint64_t bufferSize (9);

  std::string trafficPattern = "UniformRandom";

  uint64_t warmupCycles = 1000;

  // the number of simulation cycles includes the warmup cycles
  uint64_t simulationCycles = 10000;

  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue<string> ("experiment", "The experiment is the study of which this trial (AKA simulation) is a member (mandatory parameter).", experiment);
  cmd.AddValue<string> ("strategy", "The strategy is the code or parameters being examined in this trial (mandatory parameter).", strategy);
  cmd.AddValue<string> ("input", "The input is the particular problem given to this trial (mandatory parameter).", input);
  cmd.AddValue<string> ("description", "The description can be used to specify the characteristics of the simulated NoC (optional parameter).", description);
  cmd.AddValue<string> ("run", "The run is a unique identifier for this trial with which it's information is tagged for identification in later analysis (mandatory parameter).", run);
  cmd.AddValue<string> ("author", "The author of this simulation (optional parameter).", author);
  cmd.AddValue<string> ("mapping-file-path", "The path to the XML file that contains the application mapping (mandatory parameter)", mappingXmlFilePath);
  cmd.AddValue<uint32_t> ("nodes", "The number of nodes from the NoC (default is 16)", numberOfNodes);
  cmd.AddValue<uint32_t> ("h-size", "How many nodes a 2D mesh has horizontally (default is 4)", hSize);
  cmd.AddValue<uint64_t> ("flit-size", "The size of a flit, in bytes (default is 32, minimum value is given by the size of the packet header)", flitSize);
  cmd.AddValue<uint64_t> ("flits-per-packet", "How many flits a packet has (default is 9, minimum value is 2)", flitsPerPacket);
//  cmd.AddValue<double> ("injection-probability", "The packet injection probability (default is 1 (maximum), minimum is zero).", injectionProbability);
  cmd.AddValue<int> ("data-packet-speedup", "The speedup used for data packets (compared to head packets) (default is 1 (minimum value) - no speedup)", dataFlitSpeedup);
  cmd.AddValue<uint64_t> ("buffer-size", "The size of the input channel buffers (measured in multiples of packet size) (default is 9)", bufferSize);
  cmd.AddValue<uint64_t> ("warmup-cycles", "The number of simulation warm-up cycles (default is 1000)", warmupCycles);
  cmd.AddValue<uint64_t> ("simulation-cycles", "The number of simulation cycles (includes the warm-up cycles, default is 10000)", simulationCycles);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("ns-3 NoC simulator for UniMap");

  NS_ASSERT_MSG (mappingXmlFilePath.size() > 0, "Please specify the file path for the XML containing the mapping!");
  NS_ASSERT_MSG (numberOfNodes % hSize == 0,
      "The number of nodes ("<< numberOfNodes
      <<") must be a multiple of the number of nodes on the horizontal axis ("
      << hSize << ")");
  NS_ASSERT_MSG (flitSize >= (uint64_t) NocHeader::HEADER_SIZE, "The flit size must be at least " << NocHeader::HEADER_SIZE << "(the packet header size)!");
  NS_ASSERT_MSG (flitsPerPacket >= 2, "At least 2 flits per packet are required!");
//  NS_ASSERT_MSG (injectionProbability >= 0 && injectionProbability <= 1, "Injection probability must be in [0,1]!");
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
  Ptr<NocHelper> noc = CreateObject<NocHelper> ();

  // set channel bandwidth to 1 flit / network clock
  // FIXME properly set the channel bandwidth
  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate ("256Gbps")));
//  noc->SetChannelAttribute ("DataRate", DataRateValue (DataRate ((uint64_t) ((flitSize * 8) / globalClock.GetSeconds ()))));
  // the channel has no propagation delay
  noc->SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0)));

// By default, we use full-duplex communication
  //  noc->SetChannelAttribute ("FullDuplex", BooleanValue (false));

// Setting the size of the input buffers, uniformly
  noc->SetInQueue ("ns3::DropTailQueue",
      "Mode", EnumValue (DropTailQueue::PACKETS),
      "MaxPackets", UintegerValue (bufferSize));

  // configure the routers
  ObjectFactory routerFactory;
  routerFactory.SetTypeId ("ns3::FourWayRouter");
//  routerFactory.SetTypeId ("ns3::IrvineLoadRouter");

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

  // setting the routing protocol
  ObjectFactory routingProtocolFactory;
  routingProtocolFactory.SetTypeId ("ns3::XyRouting");
//  routingProtocolFactory.Set ("RouteXFirst", BooleanValue (false));

//  routingProtocolFactory.SetTypeId ("ns3::SlbRouting");
//  routingProtocolFactory.Set ("LoadThreshold", IntegerValue (30));

//  routingProtocolFactory.SetTypeId ("ns3::SoRouting");

  // setting the switching mechanism
  ObjectFactory switchingProtocolFactory;
//  switchingProtocolFactory.SetTypeId ("ns3::SafSwitching");
//  switchingProtocolFactory.SetTypeId ("ns3::VctSwitching");
  switchingProtocolFactory.SetTypeId ("ns3::WormholeSwitching");

  // installing the topology
  NetDeviceContainer devs = noc->Install2DMesh (nodes, hSize,
      routerFactory,
      routingProtocolFactory,
      switchingProtocolFactory);
  // done with installing the topology

  NS_LOG_INFO ("Create CTG based Applications.");

  try
    {
      // parse the mapping XML
      auto_ptr<mappingType>
          theMappingType (
              research::noc::application_mapping::unified_framework::schema::mapping::mapping (
                  mappingXmlFilePath, flags::dont_validate));
      NS_LOG_LOGIC ("Mapping XML file path is " << mappingXmlFilePath);

      string ctgFilePath = mappingXmlFilePath.substr (0,
          mappingXmlFilePath.find_last_of (FILE_SEPARATOR));

      string apcgXmlFilePath = ctgFilePath + FILE_SEPARATOR + "apcg-"
          + theMappingType->apcg () + ".xml";
      NS_LOG_LOGIC ("APCG XML file path is " << apcgXmlFilePath);

      // parse the APCG XML
      auto_ptr<apcgType>
          theApcgType (
              research::noc::application_mapping::unified_framework::schema::apcg::apcg (
                  apcgXmlFilePath, flags::dont_validate));

      NS_ASSERT_MSG (theMappingType->apcg () == theApcgType->id (),
          "Mapping XML says the APCG ID is " << theMappingType->apcg ()
          << " but, APCG XML says the APCG ID is " << theApcgType->id () << "!");

      string ctgXmlFilePath = ctgFilePath + FILE_SEPARATOR + "ctg-"
          + theApcgType->ctg () + ".xml";
      NS_LOG_LOGIC ("CTG XML file path is " << ctgXmlFilePath);

      // parse the CTG XML
      auto_ptr<ctgType>
          theCtgType (
              research::noc::application_mapping::unified_framework::schema::ctg::ctg (
                  ctgXmlFilePath, flags::dont_validate));

      NS_ASSERT_MSG (theApcgType->ctg () == theCtgType->id (),
          "APCG XML says the CTG ID is " << theApcgType->ctg ()
          << " but, CTG XML says the CTG ID is " << theCtgType->id () << "!");

      // extract all required information from all XMLs
      string coresFilePath = ctgFilePath.substr (0, ctgFilePath.find_last_of (
          FILE_SEPARATOR) + 1) + "cores";

      string tasksFilePath = ctgFilePath + FILE_SEPARATOR + "tasks";

      for (mappingType::map_const_iterator i (theMappingType->map ().begin ()); i
          != theMappingType->map ().end (); i++)
        {
          mapType theMapType = *i;
          NS_LOG_INFO ("\t node: " << theMapType.node ());
          NS_LOG_INFO ("\t core: " << theMapType.core ());

          string coreXmlFilePath = coresFilePath + FILE_SEPARATOR + "core-"
              + theMapType.core ().get () + ".xml";
          NS_LOG_LOGIC ("IP core XML file path is " << coreXmlFilePath);

          auto_ptr<
              research::noc::application_mapping::unified_framework::schema::core::coreType>
              theCoreType (
                  research::noc::application_mapping::unified_framework::schema::core::core (
                      coreXmlFilePath, flags::dont_validate));
          NS_ASSERT_MSG (theMapType.core ().get () == theCoreType->ID (), "APCG XML says the core ID is " << theMapType.core ()
              << " but, core XML says the ID is " << theCoreType->ID () << "!");
          NS_LOG_INFO ("\t (name: " << theCoreType->name () << " frequency: "
              << theCoreType->frequency () << " (Hz) height: "
              << theCoreType->height () << " (mm) width: "
              << theCoreType->width () << " (mm) idle power: "
              << theCoreType->idlePower () << " (W) )");

          // Note that I am dereferencing theApcgType. If I am working with the (auto) pointer,
          // at the second iteration theApcgType becomes NULL and I don't know why...
          apcgType::core_type theApcgCoreType = findCoreInApcg (*theApcgType,
              theMapType.core ().get ());
          NS_ASSERT_MSG (theApcgCoreType.id () != "",
              "Error: couldn't find core with ID " << theMapType.core () << " in the APCG XML!");

          list<NocCtgApplication::TaskData> taskList;
          list<NocCtgApplication::DependentTaskData> taskSenderList;
          list<NocCtgApplication::DependentTaskData> taskReceiverList;

          for (apcgType::core_type::task_const_iterator i (theApcgCoreType.task ().begin ()); i
              != theApcgCoreType.task ().end (); i++)
            {
              NocCtgApplication::TaskData* taskData;

              apcgType::core_type::task_type theApcgTaskType = *i;
              NS_LOG_INFO ("\t\t task: " << theApcgTaskType.id ());

              string taskXmlFilePath = tasksFilePath + FILE_SEPARATOR + "task-"
                  + theApcgTaskType.id() + ".xml";
              NS_LOG_LOGIC ("Task XML file path is " << taskXmlFilePath);

              auto_ptr<research::noc::application_mapping::unified_framework::schema::task::taskType> theTaskType (
                  research::noc::application_mapping::unified_framework::schema::task::task (taskXmlFilePath,
                      flags::dont_validate));
              NS_ASSERT_MSG (theApcgTaskType.id () == theTaskType->ID (), "APCG XML says the task ID is "
                  << theApcgTaskType.id () << " but, task XML says the ID is " << theTaskType->ID () << "!");
              NS_LOG_INFO ("\t\t (name: " << theTaskType->name () << " type: "
                  << theTaskType->type ());

              for (research::noc::application_mapping::unified_framework::schema::core::coreType::task_const_iterator i (
                  theCoreType->task ().begin ()); i != theCoreType->task ().end (); i++)
                {
                  research::noc::application_mapping::unified_framework::schema::core::coreType::task_type theCoreTaskType = *i;
                  if (theTaskType->type () == theCoreTaskType.type ())
                    {
                      NS_LOG_INFO ("\t\t execution time: " << theCoreTaskType.execTime ()
                          << " (s) " << "power:" << theCoreTaskType.power ()
                          << " (W) )");
                      taskData = new NocCtgApplication::TaskData (theApcgTaskType.id (), Seconds (
                          theCoreTaskType.execTime ().get ()));
                      break;
                    }
                }
              taskList.insert (taskList.end (), *taskData);

              // build the task source and destination lists
              for (ctgType::communication_const_iterator i (theCtgType->communication ().begin ()); i
                  != theCtgType->communication ().end (); i++)
                {
                  ctgType::communication_type theCommunicationType = *i;

                  if (theCommunicationType.source ().id () == taskData->GetId ()
                      || theCommunicationType.destination ().id () == taskData->GetId ())
                    {
                      NS_LOG_INFO ("\tsource task: " << theCommunicationType.source ().id ());
                      for (research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_const_iterator
                          i (theCommunicationType.source ().deadline ().begin ()); i
                          != theCommunicationType.source ().deadline ().end (); i++)
                        {
                          research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_type
                              deadline = *i;
                          NS_LOG_INFO ("\t\tdeadline: " << deadline.type () << " at " << deadline << " (ms)");
                        }
                      NS_LOG_INFO ("\tdestination task: " << theCommunicationType.destination ().id ());

                      for (research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_const_iterator
                          i (theCommunicationType.destination ().deadline ().begin ()); i
                          != theCommunicationType.destination ().deadline ().end (); i++)
                        {
                          research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_type
                              deadline = *i;
                          NS_LOG_INFO ("\t\tdeadline: " << deadline.type () << " at " << deadline << " (ms)");
                        }

                      NS_LOG_INFO ("\tcommunication volume: " << theCommunicationType.volume () << " (bits)");

                      uint32_t sourceNodeId;
                      if (!fromString<uint32_t> (sourceNodeId, findNodeIdForTask (*theApcgType, *theMappingType,
                          theCommunicationType.source ().id ()), std::dec))
                        {
                          NS_LOG_ERROR ("The source node ID is not a number");
                        }

                      uint32_t destinationNodeId;
                      if (!fromString<uint32_t> (destinationNodeId, findNodeIdForTask (*theApcgType, *theMappingType,
                          theCommunicationType.destination ().id ()), std::dec))
                        {
                          NS_LOG_ERROR ("The destination node ID is not a number");
                        }

                      NocCtgApplication::DependentTaskData dependentTaskData = NocCtgApplication::DependentTaskData (
                          theCommunicationType.source ().id (), sourceNodeId, theCommunicationType.volume (),
                          theCommunicationType.destination ().id (), destinationNodeId);

                      if (theCommunicationType.destination ().id () == taskData->GetId ())
                        {
                          NS_LOG_DEBUG ("Inserting in the task sender list");

                          taskSenderList.insert (taskSenderList.end (), dependentTaskData);
                        }
                      if (theCommunicationType.source ().id () == taskData->GetId ())
                        {
                          NS_LOG_DEBUG ("Inserting in the task receiver list");

                          taskReceiverList.insert (taskReceiverList.end (), dependentTaskData);
                        }
                    }
                }
            }

          NocCtgApplicationHelper nocCtgAppHelper (nodes, devs, hSize, taskList, taskSenderList, taskReceiverList);
          nocCtgAppHelper.SetAttribute ("Period", TimeValue (Seconds (theCtgType->period ().get ())));
          nocCtgAppHelper.SetAttribute ("Iterations", UintegerValue (1));
          nocCtgAppHelper.SetAttribute ("FlitSize", UintegerValue (flitSize));
          nocCtgAppHelper.SetAttribute ("NumberOfFlits", UintegerValue (flitsPerPacket));
          //      nocCtgAppHelper.SetAttribute ("MaxFlits", UintegerValue (100));

          uint32_t nodeId;
          if (!fromString<uint32_t> (nodeId, theMapType.node (), std::dec))
            {
              NS_LOG_ERROR ("The node ID " << theMapType.node () << " is not a number");
            }

          ApplicationContainer apps = nocCtgAppHelper.Install (nodes.Get (nodeId)); // source
          double startTime = 0.0;
          apps.Start (Seconds (startTime));
          //      apps.Stop (Seconds (10.0));
          // the application can also be limited by MaxPackets (the two ways of ending the application are equivalent)
          //      apps.Stop (Seconds (simulationCycles * globalClock + startTime)); // stop = simulationCycles * globalClock + start

        }
    }
  catch (const xml_schema::exception& e)
    {
      NS_LOG_ERROR (e);
      return 1;
    }

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
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

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
