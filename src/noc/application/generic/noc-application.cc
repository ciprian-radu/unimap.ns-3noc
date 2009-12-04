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

#include "noc-application.h"
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
#include "ns3/trace-source-accessor.h"
#include "ns3/enum.h"
#include <cstdlib>
#include <bitset>
#include "stdio.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("NocApplication");

using namespace std;

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocApplication);

  std::string
  NocApplication::TrafficPatternToString(TrafficPattern t)
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

  TypeId
  NocApplication::GetTypeId(void)
  {
    static TypeId
        tid = TypeId("ns3::NocApplication")
            .SetParent<Application> ()
            .AddConstructor<NocApplication> ()
            .AddAttribute("DataRate", "The data rate",
                DataRateValue(DataRate("500kb/s")), MakeDataRateAccessor(
                    &NocApplication::m_dataRate), MakeDataRateChecker())
            .AddAttribute("HSize", "The horizontal size of a 2D mesh (how many nodes can be put on a line)."
                " The vertical size of the 2D mesh is given by number of nodes", UintegerValue(4),
                MakeUintegerAccessor(&NocApplication::m_hSize),
                MakeUintegerChecker<uint32_t> (2))
            .AddAttribute("PacketSize", "The size of packets sent (in Bytes)", UintegerValue(8 + 512), // header + payload
                MakeUintegerAccessor(&NocApplication::m_pktSize),
                MakeUintegerChecker<uint32_t> (1))
            .AddAttribute("MaxBytes",
                "The total number of bytes to send. Once these bytes are sent, "
                "no packet is sent again. The value zero means that there is no limit.",
                UintegerValue(0), MakeUintegerAccessor(&NocApplication::m_maxBytes),
                MakeUintegerChecker<uint32_t> ())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                MakeTraceSourceAccessor(&NocApplication::m_txTrace))
            .AddAttribute("TrafficPattern",
                "The traffic pattern which will be used by this application",
                EnumValue (BIT_COMPLEMENT), MakeEnumAccessor (&NocApplication::m_trafficPattern),
                MakeEnumChecker (DESTINATION_SPECIFIED, TrafficPatternToString(DESTINATION_SPECIFIED),
                                 UNIFORM_RANDOM, TrafficPatternToString(UNIFORM_RANDOM),
                                 BIT_MATRIX_TRANSPOSE, TrafficPatternToString(BIT_MATRIX_TRANSPOSE),
                                 BIT_COMPLEMENT, TrafficPatternToString(BIT_COMPLEMENT),
                                 BIT_REVERSE, TrafficPatternToString(BIT_REVERSE)))
            .AddAttribute("Destination", "The ID of the destination node "
                "(must be specified for a destination specified traffic pattern)", UintegerValue(0),
                MakeUintegerAccessor(&NocApplication::m_destinationNodeId),
                MakeUintegerChecker<uint32_t> (0))
            ;
    return tid;
  }

  NocApplication::NocApplication()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_connected = false;
    m_residualBits = 0;
    m_lastStartTime = Seconds(0);
    m_totBytes = 0;
    m_trafficPattern = BIT_COMPLEMENT;
  }

  NocApplication::~NocApplication()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NocApplication::SetMaxBytes(uint32_t maxBytes)
  {
    NS_LOG_FUNCTION (this << maxBytes);
    m_maxBytes = maxBytes;
  }

  void NocApplication::SetNetDeviceContainer(NetDeviceContainer devices)
  {
    NS_LOG_FUNCTION_NOARGS();
    m_devices = devices;
  }

  void NocApplication::SetNodeContainer(NodeContainer nodes)
  {
    NS_LOG_FUNCTION_NOARGS();
    m_nodes = nodes;
  }

  void
  NocApplication::DoDispose(void)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Application::DoDispose();
  }

  // Application Methods
  void
  NocApplication::StartApplication() // Called at time specified by Start
  {
    NS_LOG_FUNCTION_NOARGS ();
    NS_LOG_DEBUG ("Using the " << TrafficPatternToString(m_trafficPattern) << " traffic pattern");

    // Ensure no pending event
    CancelEvents();
    m_connected = true;

    if (m_connected)
      {
        ScheduleStartEvent();
      }
  }

  void
  NocApplication::StopApplication() // Called at time specified by Stop
  {
    NS_LOG_FUNCTION_NOARGS ();

    CancelEvents();
  }

  void
  NocApplication::CancelEvents()
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (m_sendEvent.IsRunning())
      { // Cancel the pending send packet event
        // Calculate residual bits since last packet sent
        Time delta(Simulator::Now() - m_lastStartTime);
        Scalar bits = delta * Scalar(m_dataRate.GetBitRate()) / Seconds(1.0);
        m_residualBits += (uint32_t) bits.GetDouble();
      }
    Simulator::Cancel(m_sendEvent);
    Simulator::Cancel(m_startEvent);
  }

  // Event handlers
  void
  NocApplication::StartSending()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_lastStartTime = Simulator::Now();
    ScheduleNextTx(); // Schedule the send packet event
  }

  void
  NocApplication::StopSending()
  {
    NS_LOG_FUNCTION_NOARGS ();
    CancelEvents();

    ScheduleStartEvent();
  }

  // Private helpers
  void
  NocApplication::ScheduleNextTx()
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
      {
        uint32_t bits = m_pktSize * 8 - m_residualBits;
        NS_LOG_LOGIC ("bits = " << bits);
        Time nextTime(Seconds(bits / static_cast<double> (m_dataRate.GetBitRate()))); // Time till next packet
        NS_LOG_LOGIC ("nextTime = " << nextTime << " (packet size = " << bits << " data rate = " << m_dataRate.GetBitRate() << ")");
        m_sendEvent = Simulator::Schedule(nextTime, &NocApplication::SendPacket, this);
      }
    else
      { // All done, cancel any pending events
        StopApplication();
      }
  }

  void
  NocApplication::ScheduleStartEvent()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_startEvent = Simulator::Schedule(Simulator::Now(), &NocApplication::StartSending, this);
  }

  void
  NocApplication::SendPacket()
  {
    NS_LOG_FUNCTION_NOARGS ();
    NS_LOG_LOGIC ("sending packet at " << Simulator::Now ());
    NS_ASSERT (m_sendEvent.IsExpired ());

    Ptr<NocNode> sourceNode = GetNode ()->GetObject<NocNode> ();
    uint32_t sourceNodeId = sourceNode->GetId ();
    NS_ASSERT_MSG (sourceNodeId < 64, "max. 64 nodes (8x8 2D mesh) are allowed by this packet format");
    uint32_t sourceX = sourceNodeId / m_hSize;
    uint32_t sourceY = sourceNodeId % m_hSize;

    uint32_t destinationX;
    uint32_t destinationY;

    double log = 0;
    if (m_hSize > 0)
      {
        log = log2(m_hSize);
      }
    uint8_t sizeX = floor(log);

    log = 0;
    if (m_nodes.GetN() / m_hSize > 0)
      {
        log = log2(m_nodes.GetN() / m_hSize);
      }
    uint8_t sizeY = floor(log);

    switch (m_trafficPattern)
      {
        case DESTINATION_SPECIFIED:
          destinationX = m_destinationNodeId / m_hSize;
          NS_LOG_DEBUG ("specified destination x = " << destinationX);
          NS_ASSERT (destinationX < m_hSize);
          destinationY = m_destinationNodeId % m_hSize;
          NS_LOG_DEBUG ("specified destination y = " << destinationY);
          NS_ASSERT (destinationY < m_nodes.GetN() / m_hSize);
          break;

        case UNIFORM_RANDOM:
          {
            UniformVariable uniformVariable;
            destinationX = uniformVariable.GetInteger(0, m_hSize - 1);
            NS_LOG_DEBUG("random destination x = " << destinationX);
            NS_ASSERT(destinationX < m_hSize);
            destinationY = uniformVariable.GetInteger(0, m_nodes.GetN() / m_hSize - 1);
            NS_LOG_DEBUG("random destination y = " << destinationY);
            NS_ASSERT(destinationY < m_nodes.GetN() / m_hSize);
          }
          break;

        case BIT_MATRIX_TRANSPOSE:
          destinationX = MatrixTransposeBits(sourceX, sizeX);
          NS_ASSERT(destinationX < m_hSize);
          destinationY = MatrixTransposeBits(sourceY, sizeY);
          NS_ASSERT(destinationY < m_nodes.GetN() / m_hSize);
          break;

        case BIT_COMPLEMENT:
          destinationX = ComplementBits(sourceX, sizeX);
          NS_ASSERT(destinationX < m_hSize);
          destinationY = ComplementBits(sourceY, sizeY);
          NS_ASSERT(destinationY < m_nodes.GetN() / m_hSize);
          break;

        case BIT_REVERSE:
          destinationX = ReverseBits(sourceX, sizeX);
          NS_ASSERT(destinationX < m_hSize);
          destinationY = ReverseBits(sourceY, sizeY);
          NS_ASSERT(destinationY < m_nodes.GetN() / m_hSize);
          break;

        default:
          break;
      }

    uint32_t destinationNodeId = destinationX * m_hSize + destinationY;
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
    NS_LOG_DEBUG ("A packet is sent from node " << sourceNodeId << " to node " << destinationNodeId);

    uint32_t relativeX = 0;
    uint32_t relativeY = 0;
    if (destinationY < sourceY)
      {
        // 0 = East; 1 = West
        relativeX = 8; // 1000 (in binary)
      }
    if (destinationX < sourceX)
      {
        // 0 = South; 1 = North
        relativeY = 8; // 1000 (in binary)
      }
    relativeX = relativeX | std::abs((int) (destinationY - sourceY));
    relativeY = relativeY | std::abs((int) (destinationX - sourceX));
    // end traffic pattern

    Ptr<NocPacket> packet = Create<NocPacket> (relativeX, relativeY, sourceX, sourceY, m_pktSize);
    m_txTrace(packet);
    sourceNode->InjectPacket (packet, destinationNode);

    m_totBytes += m_pktSize;
    m_lastStartTime = Simulator::Now();
    m_residualBits = 0;
    ScheduleNextTx();
  }

  uint32_t
  NocApplication::MatrixTransposeBits (uint32_t number)
  {
    NS_LOG_FUNCTION (number);

    std::bitset<32> b(number);
    double log = 0;
    if (number > 0)
      {
        log = log2(number);
      }

    return MatrixTransposeBits(number, floor (log + 1));
  }

  uint32_t
  NocApplication::MatrixTransposeBits (uint32_t number, uint8_t size)
  {
    NS_LOG_FUNCTION (number << (int) size);
    NS_ASSERT_MSG(size >= 1 && size <= 32, "The size must be <= 1 and <= 32");

    std::bitset<32> b(number);
    std::string binary(b.to_string().substr(32 - size));
    std::bitset<32> bits(binary);
    for (unsigned int i = 0; i < binary.size() / 2; ++i)
      {
      bool leftBit = bits.test (i);
      bool rightBit = bits.test (binary.size () / 2 + i);
      bits.set (i, rightBit);
      bits.set (binary.size () / 2 + i, leftBit);
      }
    uint32_t transposedNumber = bits.to_ulong();
    NS_LOG_DEBUG(number << " " << b.to_string().substr(32 - size) << " "
        << transposedNumber << " " << bits.to_string());

    return transposedNumber;
  }

  uint32_t
  NocApplication::ComplementBits (uint32_t number)
  {
    NS_LOG_FUNCTION (number);

    std::bitset<32> b(number);
    double log = 0;
    if (number > 0)
      {
        log = log2(number);
      }

    return ComplementBits(number, floor (log + 1));
  }

  uint32_t
  NocApplication::ComplementBits (uint32_t number, uint8_t size)
  {
    NS_LOG_FUNCTION (number << (int) size);
    NS_ASSERT_MSG(size >= 1 && size <= 32, "The size must be <= 1 and <= 32");

    std::bitset<32> b(number);
    std::string binary(b.to_string().substr(32 - size));
    std::bitset<32> bits(binary);
    for (unsigned int i = 0; i < binary.size(); ++i)
      {
        bits.flip(i);
      }
    uint32_t reversedNumber = bits.to_ulong();
    NS_LOG_DEBUG(number << " " << b.to_string().substr(32 - size) << " "
        << reversedNumber << " " << bits.to_string());

    return reversedNumber;
  }

  uint32_t
  NocApplication::ReverseBits (uint32_t number)
  {
    NS_LOG_FUNCTION (number);

    std::bitset<32> b(number);
    double log = 0;
    if (number > 0)
      {
        log = log2(number);
      }

    return ReverseBits (number, floor (log + 1));
  }

  uint32_t
  NocApplication::ReverseBits (uint32_t number, uint8_t size)
  {
    NS_LOG_FUNCTION (number << (int) size);
    NS_ASSERT_MSG(size >= 1 && size <= 32, "The size must be <= 1 and <= 32");

    std::bitset<32> b(number);
    std::string binary(b.to_string().substr(32 - size));
    std::bitset<32> bits(binary);
    for (unsigned int i = 0; i < binary.size() / 2; ++i)
      {
        bool leftBit = bits.test (i);
        bool rightBit = bits.test (binary.size () - i - 1);
        bits.set (i, rightBit);
        bits.set (binary.size () - i - 1, leftBit);
      }
    uint32_t reversedNumber = bits.to_ulong();
    NS_LOG_DEBUG(number << " " << b.to_string().substr(32 - size) << " "
        << reversedNumber << " " << bits.to_string());

    return reversedNumber;
  }

} // Namespace ns3
