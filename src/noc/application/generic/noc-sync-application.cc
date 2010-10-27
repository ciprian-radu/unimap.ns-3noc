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

#include "noc-sync-application.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/noc-node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-net-device.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/noc-registry.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/enum.h"
#include <cstdlib>
#include <bitset>
#include "stdio.h"
#include "ns3/config.h"
#include "ns3/noc-packet-tag.h"

NS_LOG_COMPONENT_DEFINE ("NocSyncApplication");

using namespace std;

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocSyncApplication);

  std::string
  NocSyncApplication::TrafficPatternToString (TrafficPattern t)
  {
    switch (t)
      {
        case DESTINATION_SPECIFIED:
          return "DestinationSpecified";

        case UNIFORM_RANDOM:
          return "UniformRandom";

        case BIT_MATRIX_TRANSPOSE:
          return "BitMatrixTranspose";

        case BIT_COMPLEMENT:
          return "BitComplement";

        case BIT_REVERSE:
          return "BitReverse";

        default:
          return "InvalidTrafficPattern";
      }
  }

  NocSyncApplication::TrafficPattern
  NocSyncApplication::TrafficPatternFromString (std::string t)
  {
    if (t == "DestinationSpecified")
      {
        return DESTINATION_SPECIFIED;
      }
    if (t == "UniformRandom")
      {
        return UNIFORM_RANDOM;
      }
    if (t == "BitMatrixTranspose")
      {
        return BIT_MATRIX_TRANSPOSE;
      }
    if (t == "BitComplement")
      {
        return BIT_COMPLEMENT;
      }
    if (t == "BitReverse")
      {
        return BIT_REVERSE;
      }
    NS_LOG_WARN ("The traffic pattern called '" << t << "' is unknown! "
        "Using the default traffic pattern (Uniform random)");
    return UNIFORM_RANDOM;
  }

  TypeId
  NocSyncApplication::GetTypeId(void)
  {
    static TypeId
        tid = TypeId("ns3::NocSyncApplication")
            .SetParent<Application> ()
            .AddConstructor<NocSyncApplication> ()
            .AddAttribute ("InjectionProbability", "The injection probability",
                DoubleValue (0.5), MakeDoubleAccessor (&NocSyncApplication::m_injectionProbability),
                MakeDoubleChecker<double> (0, 1))
            .AddAttribute("HSize", "The horizontal size of a 2D mesh (how many nodes can be put on a line)."
                " The vertical size of the 2D mesh is given by number of nodes", UintegerValue(4),
                MakeUintegerAccessor(&NocSyncApplication::m_hSize),
                MakeUintegerChecker<uint32_t> (2))
            .AddAttribute("PacketSize", "The size of data packets sent (in Bytes). "
                "For head packets, the size of the header is not included.", UintegerValue(512),
                MakeUintegerAccessor(&NocSyncApplication::m_pktSize),
                MakeUintegerChecker<uint32_t> (1))
            .AddAttribute("NumberOfPackets", "How many packets a message will have.", UintegerValue(3),
                MakeUintegerAccessor(&NocSyncApplication::m_numberOfPackets),
                MakeUintegerChecker<uint32_t> (1))
            .AddAttribute("MaxBytes",
                "The total number of bytes to send. Once these bytes are sent, "
                "no packet is sent again. The value zero means that there is no limit. "
                "Note that if you also set MaxPackets, both constraints must be met for the application to stop.",
                UintegerValue(0), MakeUintegerAccessor(&NocSyncApplication::m_maxBytes),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute("MaxPackets",
                "The maximum number of packets (head and data) that could be injected. "
                "We say 'could' because the injection probability is considered. "
                "The value zero means that there is no limit. "
                "Note that if you also set MaxBytes, both constraints must be met for the application to stop.",
                UintegerValue(0), MakeUintegerAccessor(&NocSyncApplication::m_maxPackets),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute("WarmupCycles",
                "How many warmup cycles are considered. During warmup cycles, no statistics are collected",
                UintegerValue(0), MakeUintegerAccessor(&NocSyncApplication::m_warmupCycles),
                MakeUintegerChecker<uint32_t> ())
            .AddTraceSource("Tx", "A new packet is created and sent",
                MakeTraceSourceAccessor(&NocSyncApplication::m_txTrace))
            .AddTraceSource("MessageInjected", "A new message was injected into the network",
                MakeTraceSourceAccessor(&NocSyncApplication::m_messageTrace))
            .AddTraceSource("PacketReceived", "A packet reached its destination",
                MakeTraceSourceAccessor(&NocSyncApplication::m_packetReceivedTrace))
            .AddAttribute("TrafficPattern",
                "The traffic pattern which will be used by this application",
                EnumValue (BIT_COMPLEMENT), MakeEnumAccessor (&NocSyncApplication::m_trafficPatternEnum),
                MakeEnumChecker (DESTINATION_SPECIFIED, TrafficPatternToString(DESTINATION_SPECIFIED),
                                 UNIFORM_RANDOM, TrafficPatternToString(UNIFORM_RANDOM),
                                 BIT_MATRIX_TRANSPOSE, TrafficPatternToString(BIT_MATRIX_TRANSPOSE),
                                 BIT_COMPLEMENT, TrafficPatternToString(BIT_COMPLEMENT),
                                 BIT_REVERSE, TrafficPatternToString(BIT_REVERSE)))
            .AddAttribute("Destination", "The ID of the destination node "
                "(must be specified for a destination specified traffic pattern)", UintegerValue(0),
                MakeUintegerAccessor(&NocSyncApplication::m_destinationNodeId),
                MakeUintegerChecker<uint32_t> (0))
            ;
    return tid;
  }

  NocSyncApplication::NocSyncApplication()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_connected = false;
    m_lastStartTime = Seconds(0);
    m_totBytes = 0;
    m_totPackets = 0;
    m_trafficPatternEnum = BIT_COMPLEMENT;
    m_currentPacketIndex = 0;
  }

  NocSyncApplication::~NocSyncApplication()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NocSyncApplication::PacketReceivedCallback (std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION ("path" << path << "packet UID" << packet->GetUid ());
    if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
      {
        NS_LOG_DEBUG ("Tracing the packet");
        m_packetReceivedTrace (packet);
      }
    else
      {
        NS_LOG_DEBUG ("Not tracing the packet");
      }
  }

  void
  NocSyncApplication::SetMaxBytes(uint32_t maxBytes)
  {
    NS_LOG_FUNCTION (this << maxBytes);
    m_maxBytes = maxBytes;
  }

  void NocSyncApplication::SetNetDeviceContainer(NetDeviceContainer devices)
  {
    NS_LOG_FUNCTION_NOARGS();
    m_devices = devices;
  }

  void NocSyncApplication::SetNodeContainer(NodeContainer nodes)
  {
    NS_LOG_FUNCTION_NOARGS();
    m_nodes = nodes;
  }

  void
  NocSyncApplication::DoDispose(void)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Application::DoDispose();
  }

  // Application Methods
  void
  NocSyncApplication::StartApplication() // Called at time specified by Start
  {
    NS_LOG_FUNCTION_NOARGS ();

    uint32_t nodeId = GetNode ()->GetId ();
    NS_LOG_DEBUG ("Tracing the packets received at node " << (int) nodeId);
    // we configure this trace here and not in the constructor, because
    // the node is not initialized yet at constructor time
    std::stringstream ss;
    ss << "/NodeList/" << nodeId << "/DeviceList/*/$ns3::NocNetDevice/Receive";
    Config::Connect (ss.str (), MakeCallback (&NocSyncApplication::PacketReceivedCallback, this));

    NS_LOG_DEBUG ("Using the " << TrafficPatternToString(m_trafficPatternEnum) << " traffic pattern");

    // Ensure no pending event
    CancelEvents();
    m_connected = true;

    if (m_connected)
      {
        ScheduleStartEvent();
      }
  }

  void
  NocSyncApplication::StopApplication() // Called at time specified by Stop
  {
    NS_LOG_FUNCTION_NOARGS ();

    CancelEvents();
  }

  void
  NocSyncApplication::CancelEvents()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Cancel(m_sendEvent);
    Simulator::Cancel(m_startEvent);
  }

  // Event handlers
  void
  NocSyncApplication::StartSending()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_lastStartTime = Simulator::Now();
    ScheduleNextTx(); // Schedule the send packet event
  }

  void
  NocSyncApplication::StopSending()
  {
    NS_LOG_FUNCTION_NOARGS ();
    CancelEvents();

    ScheduleStartEvent();
  }

  Time
  NocSyncApplication::GetGlobalClock () const
  {
    TimeValue timeValue;
    NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
    Time globalClock = timeValue.Get ();
    NS_ASSERT_MSG (!globalClock.IsZero (), "A global clock must be set!");

    return globalClock;
  }

  // Private helpers
  void
  NocSyncApplication::ScheduleNextTx()
  {
    NS_LOG_FUNCTION_NOARGS ();

    if ((m_maxBytes == 0 || (m_maxBytes > 0 && m_totBytes < m_maxBytes))
        && (m_maxPackets == 0 || (m_maxPackets > 0 && m_totPackets < m_maxPackets)))
      {
//        int speedup = 1;
//        if (m_currentPacketIndex != 0)
//          {
//            // a data packet will be sent
//            IntegerValue dataFlitSpeedup;
//            Ptr<NocRegistry> nocRegistry = NocRegistry::GetInstance ();
//            nocRegistry->GetAttribute ("DataPacketSpeedup", dataFlitSpeedup);
//            speedup = dataFlitSpeedup.Get ();
//            NS_LOG_DEBUG ("Data packet speedup is " << speedup);
//          }
//        else
//          {
//            NS_LOG_DEBUG ("Head packet speedup is " << speedup);
//          }

        Time globalClock = GetGlobalClock ();
//        Time sendAtTime = globalClock / Scalar (speedup);
        Time sendAtTime = globalClock;
        if (m_totBytes == 0)
          {
            // force the first event to occur at time zero
            sendAtTime = Seconds (0);
          }
        NS_LOG_DEBUG ("Schedule event (packet injection) to occur at time "
            << (Simulator::Now () + sendAtTime).GetSeconds () << " seconds");
        m_sendEvent = Simulator::Schedule (sendAtTime, &NocSyncApplication::SendPacket, this);
      }
    else
      { // All done, cancel any pending events
        NS_LOG_DEBUG ("Stopping the application");
        NS_LOG_DEBUG ("maxBytes = " << m_maxBytes << " totBytes = " << m_totBytes);
        NS_LOG_DEBUG ("maxPackets = " << m_maxPackets << " totPackets = " << m_totPackets);
        StopApplication();
      }
  }

  void
  NocSyncApplication::ScheduleStartEvent()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_startEvent = Simulator::Schedule(Simulator::Now(), &NocSyncApplication::StartSending, this);
  }

  void
  NocSyncApplication::SendPacket()
  {
    NS_LOG_FUNCTION_NOARGS ();
    NS_LOG_LOGIC ("sending packet at " << Simulator::Now ());
    NS_ASSERT (m_sendEvent.IsExpired ());

    Ptr<NocNode> sourceNode = GetNode ()->GetObject<NocNode> ();
    uint32_t sourceNodeId = sourceNode->GetId ();
    uint32_t sourceX = sourceNodeId % m_hSize;
    uint32_t sourceY = sourceNodeId / m_hSize;
    NS_LOG_DEBUG ("source X = " << sourceX);
    NS_LOG_DEBUG ("source Y = " << sourceY);

    uint32_t destinationX;
    uint32_t destinationY;

    double log = 0;
    if (m_hSize > 0)
      {
        log = log2(m_hSize);
      }
    uint8_t sizeX = (uint8_t)floor(log);

    log = 0;
    if (m_nodes.GetN() / m_hSize > 0)
      {
        log = log2(m_nodes.GetN() / m_hSize);
      }
    uint8_t sizeY = (uint8_t)floor(log);

    switch (m_trafficPatternEnum)
      {
        case DESTINATION_SPECIFIED:
          destinationX = m_destinationNodeId % m_hSize;
          NS_LOG_DEBUG ("specified destination x = " << destinationX);
          NS_ASSERT (destinationX < m_hSize);
          destinationY = m_destinationNodeId / m_hSize;
          NS_LOG_DEBUG ("specified destination y = " << destinationY);
          NS_ASSERT (destinationY < m_nodes.GetN() / m_hSize);
          break;

        case UNIFORM_RANDOM:
          if (m_currentPacketIndex == 0)
            {
              m_uniformDestinationX = m_trafficPattern.GetUniformRandomNumber (0, m_hSize - 1);
              m_uniformDestinationY = m_trafficPattern.GetUniformRandomNumber (0, m_nodes.GetN() / m_hSize - 1);
//              m_uniformDestinationX = rand () % m_hSize;
//              m_uniformDestinationY = rand () % (m_nodes.GetN() / m_hSize);
            }
          destinationX = m_uniformDestinationX;
          NS_LOG_DEBUG("random destination x = " << destinationX);
          NS_ASSERT (destinationX < m_hSize);
          destinationY = m_uniformDestinationY;
          NS_LOG_DEBUG("random destination y = " << destinationY);
          NS_ASSERT (destinationY < m_nodes.GetN() / m_hSize);
          break;

        case BIT_MATRIX_TRANSPOSE:
          destinationX = ns3::TrafficPattern::MatrixTransposeBits (sourceX, sizeX);
          NS_ASSERT (destinationX < m_hSize);
          destinationY = ns3::TrafficPattern::MatrixTransposeBits (sourceY, sizeY);
          NS_ASSERT (destinationY < m_nodes.GetN() / m_hSize);
          break;

        case BIT_COMPLEMENT:
          destinationX = ns3::TrafficPattern::ComplementBits (sourceX, sizeX);
          NS_ASSERT (destinationX < m_hSize);
          destinationY = ns3::TrafficPattern::ComplementBits (sourceY, sizeY);
          NS_ASSERT (destinationY < m_nodes.GetN() / m_hSize);
          break;

        case BIT_REVERSE:
          destinationX = ns3::TrafficPattern::ReverseBits (sourceX, sizeX);
          NS_ASSERT (destinationX < m_hSize);
          destinationY = ns3::TrafficPattern::ReverseBits (sourceY, sizeY);
          NS_ASSERT (destinationY < m_nodes.GetN() / m_hSize);
          break;

        default:
          break;
      }

    uint32_t destinationNodeId = destinationY * m_hSize + destinationX;
    Ptr<NocNode> destinationNode;
    for (NetDeviceContainer::Iterator i = m_devices.Begin(); i
        != m_devices.End(); ++i)
      {
        Ptr<Node> tmpNode = (*i)->GetNode();
        if (destinationNodeId == tmpNode->GetId())
          {
            destinationNode = tmpNode->GetObject<NocNode> ();
            break;
          }
      }
    if (sourceNodeId == destinationNodeId)
      {
        NS_LOG_LOGIC ("Trying to send a message from node " << sourceNodeId << " to node "
            << destinationNodeId << ". Aborting because source and destination nodes are the same.");
        if (m_trafficPatternEnum == UNIFORM_RANDOM)
          {
            ScheduleNextTx();
          }
      }
    else
      {
        NS_LOG_LOGIC ("A packet is sent from node " << sourceNodeId << " to node " << destinationNodeId);

        uint32_t relativeX = 0;
        uint32_t relativeY = 0;
        if (destinationX < sourceX)
          {
            // 0 = East; 1 = West
            relativeX = NocHeader::DIRECTION_BIT_MASK;
          }
        if (destinationY < sourceY)
          {
            // 0 = South; 1 = North
            relativeY = NocHeader::DIRECTION_BIT_MASK;
          }
        relativeX = relativeX | std::abs ((int) (destinationX - sourceX));
        relativeY = relativeY | std::abs ((int) (destinationY - sourceY));
        // end traffic pattern

        NS_ASSERT_MSG (m_numberOfPackets >= 1,
            "The number of packets must be at least 1 (the head packet) but it is " << m_numberOfPackets);
        if (m_currentPacketIndex == 0)
          {
            int randomValue = rand ();
            randomValue = randomValue % 100 + 1; // 1 .. 100
            if (randomValue <= m_injectionProbability * 100)
              {
                NS_LOG_LOGIC ("A new message is injected into the network (injection probability is "
                    << m_injectionProbability << ")");

                m_currentHeadPacket = Create<NocPacket> (relativeX, relativeY, sourceX,
                    sourceY, m_numberOfPackets - 1, m_pktSize);
                NS_LOG_LOGIC ("Preparing to inject packet " << *m_currentHeadPacket);
                if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
                  {
                    m_txTrace (m_currentHeadPacket);
                  }
                sourceNode->InjectPacket (m_currentHeadPacket, destinationNode);
                m_currentPacketIndex++;
              }
            else
              {
                NS_LOG_LOGIC ("A new message is not injected into the network (injection probability is "
                    << m_injectionProbability << ")");
              }
          }
        else
          {
            bool isTail = false;
            if (m_currentPacketIndex + 1 == m_numberOfPackets)
              {
                isTail = true;
                NS_LOG_DEBUG ("About to inject a tail packet");
              }
            else
              {
                NS_LOG_DEBUG ("About to inject a data packet");
              }
            Ptr<NocPacket> dataPacket = Create<NocPacket> (m_currentHeadPacket->GetUid (), m_pktSize, isTail);
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_txTrace (dataPacket);
              }
            sourceNode->InjectPacket (dataPacket, destinationNode);
            m_currentPacketIndex++;
          }
        if (m_currentPacketIndex == m_numberOfPackets)
          {
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_messageTrace (m_currentHeadPacket);
              }
              m_currentPacketIndex = 0;
          }

        m_totBytes += m_pktSize;
        m_totPackets ++;
        m_lastStartTime = Simulator::Now ();
        ScheduleNextTx();
      }
  }

} // Namespace ns3
