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

#include "noc-net-device.h"
#include "ns3/noc-channel.h"
#include "ns3/noc-node.h"
#include "ns3/noc-packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/integer.h"
#include "ns3/noc-registry.h"
#include "ns3/pointer.h"

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
    m_channel (0), m_node (0), m_mtu (0xffff), m_ifIndex (0), m_routingDirection (0), m_routingDimension (-1)
  {
    m_lastScheduledEvent = PicoSeconds (0);
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
    NS_LOG_DEBUG ("NoC net device with address " << m_address << " received the packet with UID "
        << (int) packet->GetUid () << " from " << from << " to send it to " << to);

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
        Ptr<NocNetDevice> receiveDevice = GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetReceiveNetDevice ();
        if (receiveDevice != 0)
          {
            if (this == receiveDevice)
              {
                NS_LOG_DEBUG ("The packet reached its destination.");
                NocPacketTag packetTag;
                packet->RemovePacketTag (packetTag);
                packetTag.SetReceiveTime (Simulator::Now ());
                packet->AddPacketTag (packetTag);
                NS_LOG_DEBUG ("Packet receive time is " << Simulator::Now ());
                m_receiveTrace (packet);
              }
            else
              {
                NS_LOG_DEBUG ("The packet reached its destination node. Forwarding it to the processing element.");
                receiveDevice->Receive (packet, Mac48Address::ConvertFrom(receiveDevice->GetAddress ()), to);
              }
          }
        else
          {
            NS_LOG_DEBUG ("The packet reached its destination.");
            m_receiveTrace (packet);
          }
      }
    else
      {
        // this packet is intended for another net device
        // ask the node to deal with this (the node talks to the router)
        NS_LOG_DEBUG ("The packet is intended for another net device.");
        Ptr<NocNode> nocNode = GetNode ()->GetObject<NocNode> ();
        PointerValue nocPointer;
        NocRegistry::GetInstance ()->GetAttribute ("NoCTopology", nocPointer);
        Ptr<NocTopology> nocTopology = nocPointer.Get<NocTopology> ();
        NS_ASSERT_MSG (nocTopology != 0, "NoC topology was not set in the NocRegistry!");
        Ptr<NocNetDevice> destinationNetDevice = nocTopology->FindNetDeviceByAddress (to);
        nocNode->Send (this, packet, destinationNetDevice->GetNode ()->GetObject<NocNode> ());
//        Simulator::ScheduleNow (&NocNode::Send, nocNode, this, packet, destinationNetDevice->GetNode ()->GetObject<NocNode> ());
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

  Ptr<const Packet>
  NocNetDevice::DequeuePacketFromInQueue ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<const Packet> dequeuedPacket = 0;
    if (m_inQueue != 0)
      {
        if (!m_inQueue->IsEmpty ())
          {
            dequeuedPacket = m_inQueue->Dequeue ();
            if (dequeuedPacket != 0)
              {
                m_pktSrcDestMap.erase (dequeuedPacket);
                NS_LOG_LOGIC ("Dequeued packet " << *dequeuedPacket << " (UID "
                    << dequeuedPacket->GetUid () << ")"
                    << " from the input queue of NoC net device with address "
                    << GetAddress () << " (queue now has " << m_inQueue->GetNPackets ()
                    << " packets)");
              }
            else
              {
                NS_LOG_LOGIC ("No packet to dequeue from the net device " << GetAddress ());
              }
          }
        else
          {
            NS_LOG_WARN ("The in queue is empty for the net device " << GetAddress ());
          }
      }
    else
      {
        NS_LOG_WARN ("No in queue is defined for the net device " << GetAddress ());
      }

    return dequeuedPacket;
  }

  Ptr<Queue>
  NocNetDevice::GetOutQueue() const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_outQueue;
  }

  bool
  NocNetDevice::BufferPacketInInQueue (Ptr<Packet> packet)
  {
    bool enqueued = false;
    if (m_inQueue != 0)
      {
        enqueued = m_inQueue->Enqueue (packet);
      }
    else
      {
        NS_LOG_WARN ("No in queue is set for net device " << GetAddress ());
      }
    if (enqueued)
      {
        NS_LOG_DEBUG ("Buffered the packet with UID " << packet->GetUid ()
            << " in the in queue of net device " << GetAddress ());
      }
    else
      {
        NS_LOG_DEBUG ("Couldn't buffer the packet with UID " << packet->GetUid ()
            << " in the in queue of net device " << GetAddress ());
      }
    return enqueued;
  }

  uint32_t
  NocNetDevice::GetInQueueNPacktes ()
  {
    uint32_t n = 0;

    if (m_inQueue != 0)
      {
        n = m_inQueue->GetNPackets ();
      }
    NS_LOG_DEBUG ("In queue contains " << n << " packets");

    return n;
  }

  uint64_t
  NocNetDevice::GetInQueueSize ()
  {
    uint64_t n = 0;

    if (m_inQueue != 0)
      {
        UintegerValue size;
        m_inQueue->GetAttribute ("MaxPackets", size);
        n = size.Get ();
      }
    NS_LOG_DEBUG ("In queue has size " << n);

    return n;
  }

  uint32_t
  NocNetDevice::GetOutQueueNPacktes ()
  {
    uint32_t n = 0;

    if (m_outQueue != 0)
      {
        n = m_outQueue->GetNPackets ();
      }
    NS_LOG_DEBUG ("Out queue contains " << n << " packets");

    return n;
  }

  uint64_t
  NocNetDevice::GetOutQueueSize ()
  {
    uint64_t n = 0;

    if (m_outQueue != 0)
      {
        UintegerValue size;
        m_outQueue->GetAttribute ("MaxPackets", size);
        n = size.Get ();
      }
    NS_LOG_DEBUG ("Out queue has size " << n);

    return n;
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
  NocNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
  {
    bool result;

    result = SendFrom (packet, m_address, dest, protocolNumber);

    return result;
  }

  bool
  NocNetDevice::SendFrom (Ptr<Packet> packet, const Address& source,
      const Address& dest, uint16_t protocolNumber)
  {
    // Note that the packet comes with two headers!!!
    // The first header is the original header (before routing)
    // The second header is the routed header
    // We use this approach only for tracing purposes
    // Tracing as sent (in the .tr file) is an event that occurs after the packet was routed but,
    // we want to put in the trace file the original header of the packet (as it was before routing)
    NocHeader originalHeader;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        packet->RemoveHeader (originalHeader);
      }

    bool result;

    Mac48Address to = Mac48Address::ConvertFrom (dest);
    Mac48Address from;
    if (m_viaNetDevice != 0)
      {
        from = Mac48Address::ConvertFrom (m_viaNetDevice->GetAddress ());
      }
    else
      {
        from = Mac48Address::ConvertFrom (source);
      }

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
            NS_LOG_LOGIC ("Cannot buffer packet " << packet << " (UID " << packet->GetUid () << ")"
                << " in the channel input buffer of the NoC net device with address " << GetAddress ());
            Drop (packet);
          }
        else
          {
            NS_LOG_DEBUG ("Enqueued packet " << *packet << " (UID " << packet->GetUid () << ") "
                << " in the input queue of NoC net device with address "
                << GetAddress () << " (queue now has " << m_inQueue->GetNPackets () << " packets)");
            m_pktSrcDestMap.insert(std::pair<Ptr<const Packet>,SrcDest> (packet, SrcDest (from, to, m_viaNetDevice)));
          }
        ProcessBufferedPackets (originalHeader, packet);
      }
    else
      {
        packetToSend = packet;
        Ptr<Packet> tracedPacket = packetToSend->Copy ();
        NocHeader removedHeader;
        NocPacketTag tag;
        tracedPacket->PeekPacketTag (tag);
        if (NocPacket::HEAD == tag.GetPacketType ())
          {
            tracedPacket->RemoveHeader (removedHeader);
          }
        if (!removedHeader.IsEmpty() && !originalHeader.IsEmpty ())
          {
            tracedPacket->AddHeader (originalHeader);
          }
        m_sendTrace (tracedPacket);

        bool canDoRouting = GetNode ()->GetObject<NocNode> ()->GetRouter ()->
            GetSwitchingProtocol ()->ApplyFlowControl (packetToSend, m_inQueue);
        if (canDoRouting)
          {
            Ptr<NocChannel> channel = m_channel;
            if (m_viaNetDevice != 0)
              {
                channel = m_viaNetDevice->GetChannel ()->GetObject<NocChannel> ();
              }
            NS_ASSERT (channel != 0);
            uint32_t deviceId = m_deviceId;
            if (m_viaNetDevice != 0)
              {
                deviceId = m_viaNetDevice->m_deviceId;
              }
            bool canSend = channel->TransmitStart (this, packetToSend, deviceId);
            if (canSend)
              {
                result = channel->Send (to, from);
                NS_LOG_LOGIC ("Packet " << packetToSend << " (UID " << packet->GetUid () << ")"
                    << " was sent to the NoC net device with address " << to);
              }
            else
              {
                Drop (packet);
                NS_LOG_LOGIC ("Cannot send packet " << packetToSend << " (UID " << packet->GetUid () << ")"
                    << " because the channel is busy");
                result = false;
              }
          }
        else
          {
            Drop (packet);
            NS_LOG_LOGIC ("The switching protocol does not allow the packet with UID "
                << packet->GetUid () << " to be sent");
            result = false;
          }
      }

    return result;
  }

  void
  NocNetDevice::ProcessBufferedPackets (NocHeader originalHeader, Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION ("net device" << GetAddress ());
    if (m_inQueue != 0)
      {
        if (!m_inQueue->IsEmpty ())
          {
            bool result = true;
            Ptr<Packet> packetToSend;

            // a copy of the packet from the in queue is required
            // (because we cannot alter the contents of the queue and we need to change the packet's header)
            packetToSend = m_inQueue->Peek ()->Copy ();
//            if (packet != 0)
//              {
//                Ptr<Packet> tracedPacket = packet->Copy ();
//                NocHeader removedHeader;
//                tracedPacket->RemoveHeader (removedHeader);
//                if (!originalHeader.IsEmpty ())
//                  {
//                    tracedPacket->AddHeader (originalHeader);
//                  }
//                m_sendTrace (tracedPacket);
//              }

            SrcDest srcDest = m_pktSrcDestMap[m_inQueue->Peek ()];
            Mac48Address from = srcDest.GetSrc();
            Mac48Address to = srcDest.GetDest();
            m_viaNetDevice = srcDest.GetViaNetDevice ();

            bool canDoRouting = true;
            Ptr<NocChannel> channel = m_channel;
            if (m_viaNetDevice != 0)
              {
                channel = m_viaNetDevice->GetChannel ()->GetObject<NocChannel> ();
              }
            NS_ASSERT (channel != 0);
            if (packet != 0)
              {
                if (channel->IsBusy (m_viaNetDevice))
                  {
                    MarkHeadPacketAsBlocked (packet);
                  }
                canDoRouting = GetNode ()->GetObject<NocNode> ()->GetRouter ()->
                    GetSwitchingProtocol ()->ApplyFlowControl (packet, m_inQueue);
              }
            if (canDoRouting)
              {
                uint32_t deviceId = m_deviceId;
                if (m_viaNetDevice != 0)
                  {
                    deviceId = m_viaNetDevice->m_deviceId;
                  }
                bool canSend = channel->TransmitStart (this, packetToSend, deviceId);
                if (canSend)
                  {
                    result = channel->Send (to, from);
                    if (result)
                      {
                        Ptr<Packet> tracedPacket = packetToSend->Copy ();
                        NocHeader removedHeader;
                        NocPacketTag tag;
                        tracedPacket->PeekPacketTag (tag);
                        if (NocPacket::HEAD == tag.GetPacketType ())
                          {
                            tracedPacket->RemoveHeader (removedHeader);
                          }
                        if (!removedHeader.IsEmpty() && !originalHeader.IsEmpty ())
                          {
                            tracedPacket->AddHeader (originalHeader);
                          }
                        NS_LOG_DEBUG ("Tracing packet with UID " << tracedPacket->GetUid ()
                            << " as being sent at time " << Simulator::Now ());
                        m_sendTrace (tracedPacket);

                        NS_LOG_LOGIC ("Packet " << *packetToSend << " (UID " << packetToSend->GetUid () << ")"
                            << " was sent to the NoC net device with address " << to);
//                        Ptr<const Packet> dequeuedPacket = m_inQueue->Dequeue ();
//                        if (dequeuedPacket != 0)
//                          {
//                            m_pktSrcDestMap.erase(dequeuedPacket);
//                          }
//                        NS_LOG_DEBUG ("Dequeued packet " << *packetToSend << " (UID " << packetToSend->GetUid () << ")"
//                            << " from the input queue of NoC net device with address "
//                            << GetAddress () << " (queue now has " << m_inQueue->GetNPackets () << " packets)");
                      }
                    else
                      {
                        NS_LOG_LOGIC ("Packet " << *packetToSend << " (UID " << packetToSend->GetUid () << ")"
                            << " was NOT sent to the NoC net device with address " << to);
                        result = false;
                      }
                  }
                else
                  {
                    NS_LOG_LOGIC ("Cannot send packet " << *packetToSend << " (UID " << packetToSend->GetUid () << ")"
                        << " because the channel is busy");
                    result = false;
                  }
              }
            else
              {
                NS_LOG_LOGIC ("The switching protocol does not allow the packet with UID "
                    << m_inQueue->Peek ()->GetUid () << " to be sent");
                result = false;
              }
            if (!m_inQueue->IsEmpty ())
              {
                TimeValue timeValue;
                NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
                Time globalClock = timeValue.Get ();
                if (globalClock.IsZero ())
                  {
                    // the channel's bandwidth is obviously expressed in bits / s
                    // however, in order to avoid losing precision, we create a PicoSeconds object (instead of a Seconds object)
                    Time tEvent = PicoSeconds ((uint64_t) channel->GetDataRate ().CalculateTxTime (m_inQueue->Peek ()->GetSize ()));
                    Time time = tEvent + channel->GetDelay ();
                    if (m_lastScheduledEvent != Simulator::Now () + time)
                      {
                        NS_LOG_LOGIC ("Schedule event (net device process buffered packets) to occur at time "
                            << Simulator::Now () + time
                            << " (net device " << GetAddress ()<< ", last scheduled event was at time "
                            << m_lastScheduledEvent << ")");
                        Simulator::Schedule (time, &NocNetDevice::ProcessBufferedPackets,
                            this, originalHeader, (Ptr<Packet>) 0);
                      }
                    else
                      {
                        NS_LOG_DEBUG ("An event was already scheduled at time "
                            << Simulator::Now () + time
                            << " (net device " << GetAddress () << ", last scheduled event was at time "
                            << m_lastScheduledEvent << ")");
                      }
                  }
                else
                  {
                    int speedup = 1;
                    NocPacketTag tag;
                    packetToSend->PeekPacketTag (tag);
                    if (NocPacket::HEAD == tag.GetPacketType ())
                      {
                        // a data packet will be sent
                        IntegerValue dataFlitSpeedup;
                        Ptr<NocRegistry> nocRegistry = NocRegistry::GetInstance ();
                        nocRegistry->GetAttribute ("DataPacketSpeedup", dataFlitSpeedup);
                        speedup = dataFlitSpeedup.Get ();
                      }
                    NS_LOG_DEBUG ("Data flit speedup is " << speedup);
                    NS_LOG_DEBUG ("Packet has UID " << packetToSend->GetUid ());

                    if (m_lastScheduledEvent != Simulator::Now () + globalClock / Scalar (speedup))
                      {
                        NS_LOG_DEBUG ("Packet has UID " << packetToSend->GetUid ());
                        NS_LOG_LOGIC ("Schedule event (net device process buffered packets) to occur at time "
                            << Simulator::Now () + globalClock / Scalar (speedup)
                            << " (net device " << GetAddress () << ", last scheduled event was at time "
                            << m_lastScheduledEvent);
                        // find the next network clock cycle
                        uint64_t clockMultiplier = 1 + (uint64_t) ceil (Simulator::Now ().GetSeconds ()
                            / globalClock.GetSeconds ()); // 1 + current clock cycle
                        Time nextClock = globalClock * Scalar (clockMultiplier) - Simulator::Now ();
                        NS_ASSERT_MSG (nextClock.IsPositive(), "Next clock is negative! next clock = " << nextClock.GetPicoSeconds () << " ps"
                            << "; globalClock = " << globalClock.GetPicoSeconds () << " ps"
                            << "; clockMultiplier = " << clockMultiplier
                            << "; Simulator::Now () = " << Simulator::Now ().GetPicoSeconds () << " ps"
                            << " (nextClock = globalClock * Scalar (clockMultiplier) - Simulator::Now ())");
                        m_lastScheduledEvent = Simulator::Now () + globalClock / Scalar (speedup);
                        // Simulator::Schedule (...) receives a relative time
                        NS_LOG_LOGIC ("Processing a new buffered flit at " << Simulator::Now () + nextClock);
                        Simulator::Schedule (nextClock, &NocNetDevice::ProcessBufferedPackets,
                            this, originalHeader, (Ptr<Packet>) 0);
                      }
                    else
                      {
                        NS_LOG_DEBUG ("An event was already scheduled at time "
                            << Simulator::Now () + globalClock / Scalar (speedup)
                            << " (net device " << GetAddress () << ", last scheduled event was at time "
                            << m_lastScheduledEvent << ")");
                      }
                  }
              }
          }
      }
  }

  void
  NocNetDevice::Drop (Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION (packet);
    NS_LOG_LOGIC ("Dropping packet with UID " << packet->GetUid () << " " << *packet);
    // For now we just use this method to trace dropped packets
  }

  void
  NocNetDevice::MarkHeadPacketAsBlocked (Ptr<Packet> packet)
  {
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
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
  NocNetDevice::MarkHeadPacketAsUnblocked (Ptr<Packet> packet)
  {
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
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

  void
  NocNetDevice::SetViaNetDevice (Ptr<NocNetDevice> viaNetDevice)
  {
    if (viaNetDevice == 0)
      {
        NS_LOG_FUNCTION ("no via net device");
      }
    else
      {
        NS_LOG_FUNCTION (viaNetDevice->GetAddress ());
      }
    m_viaNetDevice = viaNetDevice;
  }

  Ptr<NocNetDevice>
  NocNetDevice::GetViaNetDevice () const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_viaNetDevice;
  }

  Ptr<Node>
  NocNetDevice::GetNode () const
  {
    return m_node;
  }

  void
  NocNetDevice::SetNode (Ptr<Node> node)
  {
    m_node = node;
  }

  void
  NocNetDevice::SetRoutingDirection (int routingDirection, uint32_t dimension)
  {
    NS_LOG_FUNCTION (routingDirection << dimension);
    m_routingDirection = routingDirection;
    m_routingDimension = dimension;
  }

  int
  NocNetDevice::GetRoutingDirection () const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_routingDirection;
  }

  int
  NocNetDevice::GetRoutingDimension () const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_routingDimension;
  }

  bool
  NocNetDevice::NeedsArp () const
  {
    return false;
  }
  void
  NocNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
  {
    m_rxCallback = cb;
  }

  void
  NocNetDevice::DoDispose ()
  {
    m_channel = 0;
    m_node = 0;
    NetDevice::DoDispose();
  }

  void
  NocNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
  {
    m_promiscCallback = cb;
  }

  bool
  NocNetDevice::SupportsSendFrom () const
  {
    return true;
  }

} // namespace ns3
