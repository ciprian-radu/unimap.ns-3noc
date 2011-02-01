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

#include "noc-faulty-net-device.h"
#include "ns3/noc-channel.h"
#include "ns3/noc-node.h"
#include "ns3/noc-packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/integer.h"
#include "ns3/noc-registry.h"

NS_LOG_COMPONENT_DEFINE ("NocFaultyNetDevice");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocFaultyNetDevice);

  TypeId
  NocFaultyNetDevice::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::NocFaultyNetDevice")
        .SetParent<NocNetDevice> ()
        .AddConstructor<NocFaultyNetDevice> ()
        .AddAttribute (
            "Faulty",
            "Indicates if a router is faulty or not",
            BooleanValue (false),
            MakeBooleanAccessor (&NocFaultyNetDevice::m_faulty),
            MakeBooleanChecker ())
        ;
    return tid;
  }

  NocFaultyNetDevice::NocFaultyNetDevice () : NocNetDevice ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  NocFaultyNetDevice::~NocFaultyNetDevice ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NocFaultyNetDevice::Receive (Ptr<Packet> packet, Mac48Address to, Mac48Address from)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNetDevice::Receive (packet, to, from);
      }
    else
      {
        NS_LOG_ERROR ("Cannot receive because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  void
  NocFaultyNetDevice::SetChannel (Ptr<NocChannel> channel)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNetDevice::SetChannel (channel);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set channel because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  void
  NocFaultyNetDevice::SetInQueue (Ptr<Queue> inQueue)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNetDevice::SetInQueue (inQueue);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set in queue because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  void
  NocFaultyNetDevice::SetOutQueue(Ptr<Queue> outQueue)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNetDevice::SetOutQueue (outQueue);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set out queue because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  bool
  NocFaultyNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
  {
    NS_LOG_FUNCTION_NOARGS ();

    bool sent = false;

    if (!m_faulty)
      {
        sent = NocNetDevice::Send (packet, dest, protocolNumber);
      }
    else
      {
        NS_LOG_ERROR ("Cannot send because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return sent;
  }

  bool
  NocFaultyNetDevice::SendFrom (Ptr<Packet> packet, const Address& source,
      const Address& dest, uint16_t protocolNumber)
  {
    NS_LOG_FUNCTION_NOARGS ();

    bool sent = false;

    if (!m_faulty)
      {
        sent = NocNetDevice::SendFrom (packet, source, dest, protocolNumber);
      }
    else
      {
        NS_LOG_ERROR ("Cannot send because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return sent;
  }

  void
  NocFaultyNetDevice::Drop (Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNetDevice::Drop (packet);
      }
    else
      {
        NS_LOG_ERROR ("Cannot drop packet because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  Ptr<Node>
  NocFaultyNetDevice::GetNode () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Node> node;

    if (!m_faulty)
      {
        node = NocNetDevice::GetNode ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get node because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return node;
  }
  void
  NocFaultyNetDevice::SetNode (Ptr<Node> node)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNetDevice::SetNode (node);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set node because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  void
  NocFaultyNetDevice::SetRoutingDirection (int routingDirection, uint32_t dimension)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNetDevice::SetRoutingDirection (routingDirection, dimension);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set routing direction because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  int
  NocFaultyNetDevice::GetRoutingDirection () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    int routingDirection;

    if (!m_faulty)
      {
        routingDirection = NocNetDevice::GetRoutingDirection ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get routing direction because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return routingDirection;
  }

  int
  NocFaultyNetDevice::GetRoutingDimension () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    int routingDimension;

    if (!m_faulty)
      {
        routingDimension = NocNetDevice::GetRoutingDimension ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get routing dimension because net device " << GetAddress () << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return routingDimension;
  }

} // namespace ns3
