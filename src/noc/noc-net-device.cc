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

#include "noc-net-device.h"
#include "noc-channel.h"
#include "ns3/node.h"
#include "noc-packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocNetDevice);

  TypeId
  NocNetDevice::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::NocNetDevice")
        .SetParent<NetDevice> ()
        .AddConstructor<NocNetDevice> ()
        .AddTraceSource ("Send",
                 "Trace source indicating a packet has been sent by this device",
                 MakeTraceSourceAccessor (&NocNetDevice::m_sendTrace))
         .AddTraceSource ("Receive",
                 "Trace source indicating a packet has been received by this device",
                 MakeTraceSourceAccessor (&NocNetDevice::m_receiveTrace))
        ;
    return tid;
  }

  NocNetDevice::NocNetDevice() :
    m_channel(0), m_node(0), m_mtu(0xffff), m_ifIndex(0)
  {
  }

  void
  NocNetDevice::Receive(Ptr<Packet> packet, uint16_t protocol,
      Mac48Address to, Mac48Address from)
  {
    NetDevice::PacketType packetType;
    if (to == m_address)
      {
        packetType = NetDevice::PACKET_HOST;
      }
    else if (to.IsBroadcast())
      {
        packetType = NetDevice::PACKET_HOST;
      }
    else if (to.IsGroup())
      {
        packetType = NetDevice::PACKET_MULTICAST;
      }
    else
      {
        packetType = NetDevice::PACKET_OTHERHOST;
      }
    m_rxCallback(this, packet, protocol, from);
    if (!m_promiscCallback.IsNull())
      {
        m_promiscCallback(this, packet, protocol, from, to, packetType);
      }

    if (packetType != NetDevice::PACKET_OTHERHOST)
      {
        m_receiveTrace(packet);
      }
  }

  void
  NocNetDevice::SetChannel(Ptr<NocChannel> channel)
  {
    m_channel = channel;
    m_channel->Add(this);
  }

  void
  NocNetDevice::SetIfIndex(const uint32_t index)
  {
    m_ifIndex = index;
  }
  uint32_t
  NocNetDevice::GetIfIndex(void) const
  {
    return m_ifIndex;
  }
  Ptr<Channel>
  NocNetDevice::GetChannel(void) const
  {
    return m_channel;
  }
  void
  NocNetDevice::SetAddress(Address address)
  {
    m_address = Mac48Address::ConvertFrom(address);
  }
  Address
  NocNetDevice::GetAddress(void) const
  {
    //
    // Implicit conversion from Mac48Address to Address
    //
    return m_address;
  }
  bool
  NocNetDevice::SetMtu(const uint16_t mtu)
  {
    m_mtu = mtu;
    return true;
  }
  uint16_t
  NocNetDevice::GetMtu(void) const
  {
    return m_mtu;
  }
  bool
  NocNetDevice::IsLinkUp(void) const
  {
    return true;
  }
  void
  NocNetDevice::AddLinkChangeCallback(Callback<void> callback)
  {
  }
  bool
  NocNetDevice::IsBroadcast(void) const
  {
    return true;
  }
  Address
  NocNetDevice::GetBroadcast(void) const
  {
    return Mac48Address("ff:ff:ff:ff:ff:ff");
  }
  bool
  NocNetDevice::IsMulticast(void) const
  {
    return false;
  }
  Address
  NocNetDevice::GetMulticast(Ipv4Address multicastGroup) const
  {
    return Mac48Address::GetMulticast(multicastGroup);
  }

  Address
  NocNetDevice::GetMulticast(Ipv6Address addr) const
  {
    return Mac48Address::GetMulticast(addr);
  }

  bool
  NocNetDevice::IsPointToPoint(void) const
  {
    return false;
  }

  bool
  NocNetDevice::IsBridge(void) const
  {
    return false;
  }

  void
  NocNetDevice::SetRoutingProtocol (Ptr<NocRoutingProtocol> protocol)
  {
//    NS_LOG_FUNCTION_NOARGS ();
    NS_ASSERT_MSG (PeekPointer (protocol->GetNocNetDevice ()) == this,
        "Routing protocol must be installed on mesh point to be useful.");
    m_routingProtocol = protocol;
  }

  Ptr<NocRoutingProtocol>
  NocNetDevice::GetRoutingProtocol () const
  {
    return m_routingProtocol;
  }

  bool
  NocNetDevice::Send(Ptr<Packet> packet, const Address& dest,
      uint16_t protocolNumber)
  {
    Mac48Address to = Mac48Address::ConvertFrom(dest);
    m_sendTrace (packet);
    return m_routingProtocol->RequestRoute (m_ifIndex, m_address, to, packet, protocolNumber, MakeCallback (
        &NocNetDevice::DoSend, this));
  }

  bool
  NocNetDevice::SendFrom(Ptr<Packet> packet, const Address& source,
      const Address& dest, uint16_t protocolNumber)
  {
    Mac48Address to = Mac48Address::ConvertFrom(dest);
    Mac48Address from = Mac48Address::ConvertFrom(source);
    m_sendTrace (packet);
    return m_routingProtocol->RequestRoute (m_ifIndex, from, to, packet, protocolNumber, MakeCallback (
        &NocNetDevice::DoSend, this));
  }

  void
  NocNetDevice::DoSend (bool success, Ptr<Packet> packet, Mac48Address src, Mac48Address dst, uint16_t protocol)
  {
    m_channel->Send(packet, protocol, dst, src, this);
  }

  Ptr<Node>
  NocNetDevice::GetNode(void) const
  {
    return m_node;
  }
  void
  NocNetDevice::SetNode(Ptr<Node> node)
  {
    m_node = node;
  }
  bool
  NocNetDevice::NeedsArp(void) const
  {
    return false;
  }
  void
  NocNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
  {
    m_rxCallback = cb;
  }

  void
  NocNetDevice::DoDispose(void)
  {
    m_channel = 0;
    m_node = 0;
    NetDevice::DoDispose();
  }

  void
  NocNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
  {
    m_promiscCallback = cb;
  }

  bool
  NocNetDevice::SupportsSendFrom(void) const
  {
    return true;
  }

} // namespace ns3
