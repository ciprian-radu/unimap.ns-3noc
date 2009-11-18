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
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/noc-packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include <cstdlib>
#include <stdlib.h>

NS_LOG_COMPONENT_DEFINE ("NocApplication");

using namespace std;

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocApplication);

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
            .AddAttribute("PacketSize", "The size of packets sent (in Bytes)", UintegerValue(512),
                MakeUintegerAccessor(&NocApplication::m_pktSize),
                MakeUintegerChecker<uint32_t> (1))
            .AddAttribute("MaxBytes",
                "The total number of bytes Installto send. Once these bytes are sent, "
                "no packet is sent again. The value zero means that there is no limit.",
                UintegerValue(0), MakeUintegerAccessor(&NocApplication::m_maxBytes),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute("Protocol",
                "The type of protocol to use.", TypeIdValue(UdpSocketFactory::GetTypeId()),
                MakeTypeIdAccessor(&NocApplication::m_tid), MakeTypeIdChecker())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                MakeTraceSourceAccessor(&NocApplication::m_txTrace));
    return tid;
  }

  NocApplication::NocApplication()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_connected = false;
    m_residualBits = 0;
    m_lastStartTime = Seconds(0);
    m_totBytes = 0;
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
        NS_LOG_LOGIC ("nextTime = " << nextTime);
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

    Ptr<Node> node = GetNode ();
    uint32_t sourceNodeId = node->GetId ();
    NS_ASSERT_MSG (sourceNodeId < 64, "max. 64 nodes (8x8 2D mesh) are allowed by this packet format");
    uint32_t sourceX = sourceNodeId / m_hSize;
    uint32_t sourceY = sourceNodeId % m_hSize;

    // FIXME this is just a traffic pattern (of many)

//    char * s;
//    itoa(sourceX, s, 2);

    uint32_t destinationX = 1;
    uint32_t destinationY = 1;
    uint32_t destinationNodeId = destinationX * m_hSize + destinationY;
    Address destinationAddress;
    for (NetDeviceContainer::Iterator i = m_devices.Begin(); i != m_devices.End(); ++i)
      {
        Ptr<Node> tmpNode = (*i)->GetNode ();
        if (destinationNodeId == tmpNode->GetId())
          {
            destinationAddress = (*i)->GetAddress();
            break;
          }
      }
    uint32_t relativeX = 0;
    uint32_t relativeY = 0;
    if (destinationX - sourceX < 0)
      {
        // 0 = East; 1 = West
        relativeX = relativeX & 0x08;
      }
    if (destinationY - sourceY < 0)
      {
        // 0 = South; 1 = North
        relativeY = relativeY & 0x08;
      }
    relativeX = relativeX & std::abs((int) (destinationX - sourceX));
    relativeY = relativeY & std::abs((int) (destinationY - sourceY));

    Ptr<NetDevice> netDevice = node->GetDevice (0);
    Address address = netDevice->GetAddress ();

    Ptr<NocPacket> packet = Create<NocPacket> (relativeX, relativeY, sourceX, sourceY, m_pktSize);
    m_txTrace(packet);
    netDevice->Send(packet, destinationAddress, 0);

    m_totBytes += m_pktSize;
    m_lastStartTime = Simulator::Now();
    m_residualBits = 0;
    ScheduleNextTx();
  }

} // Namespace ns3
