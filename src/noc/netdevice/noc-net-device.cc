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
#include "ns3/noc-channel.h"
#include "ns3/node.h"
#include "ns3/noc-packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"

NS_LOG_COMPONENT_DEFINE ("NocNetDevice");

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
        .AddAttribute ("InQueue",
                 "A queue to use as the input channel buffer of this net device.",
                 PointerValue (),
                 MakePointerAccessor (&NocNetDevice::m_inQueue),
                 MakePointerChecker<Queue> ())
        .AddAttribute ("OutQueue",
                 "A queue to use as the input channel buffer of this net device.",
                 PointerValue (),
                 MakePointerAccessor (&NocNetDevice::m_outQueue),
                 MakePointerChecker<Queue> ())
        ;
    return tid;
  }

  NocNetDevice::NocNetDevice() :
    m_channel(0), m_node(0), m_mtu(0xffff), m_ifIndex(0), m_nocHelper(0), m_routingDirection(0)
  {
  }

  NocNetDevice::~NocNetDevice()
  {
    m_inQueue = 0;
    m_outQueue = 0;
  }

  void
  NocNetDevice::Receive(Ptr<Packet> packet, Mac48Address to, Mac48Address from)
  {
    int protocol = 0; // TODO we use no more than one protocol for now
    NS_LOG_DEBUG("NoC net device with address " << m_address << " received a packet from " << from
        << " to send it to " << to);

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
        NS_LOG_DEBUG ("The packet reached it's destination.");
        m_receiveTrace(packet);
      }
    else
      {
        // this packet is intended for another net device
        // ask the node to deal with this (the node talks to the router)
        NS_LOG_DEBUG ("The packet is intended for another net device.");
        Ptr<NocNode> nocNode = GetNode ()->GetObject<NocNode> ();
        Ptr<NocNetDevice> destinationNetDevice = GetNocHelper ()->FindNetDeviceByAddress (to);
        nocNode->Send (this, packet, destinationNetDevice->GetNode ()->GetObject<NocNode> ());
      }
  }

  void
  NocNetDevice::SetChannel(Ptr<NocChannel> channel)
  {
    m_channel = channel;
    m_deviceId = m_channel->Add(this);
  }

  void
  NocNetDevice::SetInQueue(Ptr<Queue> inQueue)
  {
    NS_LOG_FUNCTION (inQueue);
    m_inQueue = inQueue;
  }

  void
  NocNetDevice::SetOutQueue(Ptr<Queue> outQueue)
  {
    NS_LOG_FUNCTION (outQueue);
    m_outQueue = outQueue;
  }

  Ptr<Queue>
  NocNetDevice::GetInQueue() const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_inQueue;
  }

  Ptr<Queue>
  NocNetDevice::GetOutQueue() const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_outQueue;
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

  bool
  NocNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
  {
    bool result;

    result = SendFrom (packet, m_address, dest, protocolNumber);

    return result;
  }

  bool
  NocNetDevice::SendFrom(Ptr<Packet> packet, const Address& source,
      const Address& dest, uint16_t protocolNumber)
  {
    bool result;

    Mac48Address to = Mac48Address::ConvertFrom(dest);
    Mac48Address from = Mac48Address::ConvertFrom(source);

    Ptr<Packet> packetToSend;
    if (m_inQueue != 0)
      {
//        if (!m_inQueue->IsEmpty ())
//          {
//            // marking the head packet as blocked should suffice
//            markHeadPacketAsBlocked (packet);
//          }

        bool enqueued = m_inQueue->Enqueue (packet);
        if (!enqueued)
          {
            NS_LOG_LOGIC ("Cannot buffer packet " << packet
                << " in the channel input buffer of the NoC net device with address " << GetAddress ());
            Drop (packet);
          }
        else
          {
            NS_LOG_DEBUG ("Enqueued packet " << packet << " (as packet " << m_inQueue->Peek () << ")"
                << " in the input queue of NoC net device with address "
                << GetAddress () << " (queue now has " << m_inQueue->GetNPackets () << " packets)");
            m_pktSrcDestMap.insert(std::pair<Ptr<const Packet>,SrcDest> (packet, SrcDest (from, to)));
          }
        ProcessBufferedPackets (packet);
      }
    else
      {
        packetToSend = packet;
        m_sendTrace (packetToSend);

        bool canDoRouting = GetNode ()->GetObject<NocNode> ()->GetRouter ()->
            GetSwitchingProtocol ()->ApplyFlowControl (packetToSend, m_inQueue);
        if (canDoRouting)
          {
            bool canSend = m_channel->TransmitStart (packetToSend, m_deviceId);
            if (canSend)
              {
                result = m_channel->Send (to, from);
                NS_LOG_LOGIC("Packet " << packetToSend << " was sent to the NoC net device with address " << to);
              }
            else
              {
                Drop (packet);
                NS_LOG_LOGIC("Cannot send packet " << packetToSend << " because the channel is busy");
                result = false;
              }
          }
        else
          {
            Drop (packet);
            NS_LOG_LOGIC ("The switching protocol does not allow the packet "
                << packet->GetUid () << " to be sent");
            result = false;
          }
      }

    return result;
  }

  void
  NocNetDevice::ProcessBufferedPackets (Ptr<Packet> packet)
  {
    if (m_inQueue != 0)
      {
        if (!m_inQueue->IsEmpty ())
          {
            bool result = true;
            Ptr<Packet> packetToSend;

            // a copy of the packet from the in queue is required
            // (because we cannot alter the contents of the queue and we need to change the packet's header)
            packetToSend = m_inQueue->Peek ()->Copy ();
            if (packet != 0)
              {
                m_sendTrace (packet);
              }
            else
              {
              // This doesn't have to be sent!

//                NS_LOG_DEBUG ("Packet is NULL");
//                m_sendTrace (packetToSend);
              }
            NS_LOG_DEBUG("Packet " << packetToSend << " is a copy of packet " << m_inQueue->Peek ());

            SrcDest srcDest = m_pktSrcDestMap[m_inQueue->Peek ()];
            Mac48Address from = srcDest.GetSrc();
            Mac48Address to = srcDest.GetDest();

            bool canDoRouting = true;
            if (packet != 0)
              {
                if (m_channel->IsBusy ())
                  {
                    markHeadPacketAsBlocked (packet);
                  }
                canDoRouting = GetNode ()->GetObject<NocNode> ()->GetRouter ()->
                    GetSwitchingProtocol ()->ApplyFlowControl (packet, m_inQueue);
              }
            if (canDoRouting)
              {
                bool canSend = m_channel->TransmitStart (packetToSend, m_deviceId);
                if (canSend)
                  {
                    result = m_channel->Send (to, from);
                    if (result)
                      {
                        NS_LOG_LOGIC("Packet " << packetToSend << " was sent to the NoC net device with address " << to);
                        Ptr<const Packet> dequeuedPacket = m_inQueue->Dequeue ();
                        if (dequeuedPacket != 0)
                          {
                            m_pktSrcDestMap.erase(dequeuedPacket);
                          }
                        NS_LOG_DEBUG ("Dequeued packet " << packetToSend << " from the input queue of NoC net device with address "
                            << GetAddress () << " (queue now has " << m_inQueue->GetNPackets () << " packets)");
                      }
                    else
                      {
                        NS_LOG_LOGIC("Packet " << packetToSend << " was NOT sent to the NoC net device with address " << to);
                        result = false;
                      }
                  }
                else
                  {
                    NS_LOG_LOGIC("Cannot send packet " << packetToSend << " because the channel is busy");
                    result = false;
                  }
              }
            else
              {
                NS_LOG_LOGIC ("The switching protocol does not allow the packet "
                    << m_inQueue->Peek ()->GetUid () << " to be sent");
                result = false;
              }
            if (!m_inQueue->IsEmpty () && result) // FIXME is the result condition required here?
              {
                Time tEvent = Seconds (m_channel->GetDataRate ().CalculateTxTime (m_inQueue->Peek ()->GetSize ()));
                // FIXME the next packet cannot be sent at exactly the "channel time"
                // At exactly that time, the channel is busy and the packet will get delayed
                // Is it correct to add a small delay here to avoid the delay created by the busy channel?
                // Could we call this small time "switching time"?
                Time time = tEvent + m_channel->GetDelay () + Seconds (0.00001);
                NS_LOG_DEBUG ("Schedule event (net device process buffered packets) to occur at time "
                    << (Simulator::Now() + time).GetSeconds () << " seconds");
                Simulator::Schedule(time, &NocNetDevice::ProcessBufferedPackets, this, (Ptr<Packet>) 0);
              }
          }
      }
  }

  void
  NocNetDevice::Drop (Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION (packet);
    // For now we just use this method to trace dropped packets
  }

  void
  NocNetDevice::markHeadPacketAsBlocked (Ptr<Packet> packet)
  {
    NocHeader header;
    packet->PeekHeader (header);
    if (!header.IsEmpty ())
      {
        NocPacketTag tag;
        packet->RemovePacketTag (tag);
        tag.SetPacketBlocked (true);
        packet->AddPacketTag (tag);

        packet->PeekPacketTag (tag);
        NS_ASSERT (tag.GetPacketBlocked() == true);
        NS_LOG_DEBUG ("The packet " << packet << " (UID "
            << packet->GetUid () << ") is marked as blocked");
      }
  }

  void
  NocNetDevice::markHeadPacketAsUnblocked (Ptr<Packet> packet)
  {
    NocHeader header;
    packet->PeekHeader (header);
    if (!header.IsEmpty ())
      {
        NocPacketTag tag;
        packet->RemovePacketTag (tag);
        tag.SetPacketBlocked (false);
        packet->AddPacketTag (tag);
        packet->PeekPacketTag (tag);

        NS_ASSERT (tag.GetPacketBlocked() == false);
        NS_LOG_DEBUG ("The packet " << packet << " (UID "
            << packet->GetUid () << ") is marked as unblocked");
      }
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

  void
  NocNetDevice::SetRoutingDirection (int routingDirection)
  {
    m_routingDirection = routingDirection;
  }

  int
  NocNetDevice::GetRoutingDirection () const
  {
    return m_routingDirection;
  }

  void
  NocNetDevice::SetNocHelper (Ptr<NocHelper> nocHelper)
  {
    m_nocHelper = nocHelper;
  }

  Ptr<NocHelper>
  NocNetDevice::GetNocHelper () const
  {
    return m_nocHelper;
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
