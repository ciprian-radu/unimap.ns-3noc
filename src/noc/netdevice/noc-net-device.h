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

#ifndef NOCNETDEVICE_H_
#define NOCNETDEVICE_H_

#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include <stdint.h>
#include <string>
#include "ns3/noc-channel.h"
#include "ns3/traced-callback.h"
#include "ns3/noc-routing-protocol.h"
#include "ns3/noc-topology.h"
#include "ns3/queue.h"
#include <map>
#include "ns3/noc-header.h"
#include <vector>

namespace ns3
{

  class NocChannel;
  class NocRoutingProtocol;
  class NocTopology;

  /**
   * \ingroup netdevice
   *
   * \brief net device for a Network on Chip (NoC)
   */
  class NocNetDevice : public NetDevice
  {
  public:

    static TypeId
    GetTypeId(void);

    NocNetDevice();

    virtual
    ~NocNetDevice();

    virtual void
    Receive(Ptr<Packet> packet, Mac48Address to, Mac48Address from);

    virtual void
    SetChannel(Ptr<NocChannel> channel);

    /**
     * Attach an input queue to this NoC net device.
     *
     * The NoC net device "owns" a queue.  This queue may be set by higher
     * level topology objects to implement a particular queueing method such as
     * DropTail or RED.
     *
     * \see Queue
     * \see DropTailQueue
     * \param queue a Ptr to the queue for being assigned to the device.
     */
    virtual void
    SetInQueue (Ptr<Queue> inQueue);

    /**
     * Attach an output queue to this NoC net device.
     *
     * The NoC net device "owns" a queue.  This queue may be set by higher
     * level topology objects to implement a particular queueing method such as
     * DropTail or RED.
     *
     * \see Queue
     * \see DropTailQueue
     * \param queue a Ptr to the queue for being assigned to the device.
     */
    virtual void
    SetOutQueue (Ptr<Queue> outQueue);

    /**
     * Buffers the specified packet in the in queue of this net device
     *
     * \param packet the packet to be buffered
     *
     * \return whether or not could be buffered
     */
    bool
    BufferPacketInInQueue (Ptr<Packet> packet);

    /**
     * \return how many packets the in queue contains
     */
    uint32_t
    GetInQueueNPacktes ();

    /**
     * \return the maximum number of packets the in queue might contain
     */
    uint64_t
    GetInQueueSize ();

    /**
     * \return how many packets the out queue contains
     */
    uint32_t
    GetOutQueueNPacktes ();

    /**
     * \return the maximum number of packets the out queue might contain
     */
    uint64_t
    GetOutQueueSize ();

    /**
     * This method should be invoked whenever a packet is dropped by the net device
     *
     * \param packet the dropped packet
     */
    virtual void
    Drop (Ptr<Packet> packet);

    // inherited from NetDevice base class.
    virtual void
    SetIfIndex(const uint32_t index);

    virtual uint32_t
    GetIfIndex(void) const;

    virtual Ptr<Channel>
    GetChannel(void) const;

    virtual void
    SetAddress(Address address);

    virtual Address
    GetAddress(void) const;

    virtual bool
    SetMtu(const uint16_t mtu);

    virtual uint16_t
    GetMtu(void) const;

    virtual bool
    IsLinkUp(void) const;

    virtual void
    SetRoutingDirection (int routingDirection, uint32_t dimension);

    virtual int
    GetRoutingDirection () const;

    virtual int
    GetRoutingDimension () const;

    virtual void
    AddLinkChangeCallback(Callback<void> callback);

    virtual bool
    IsBroadcast(void) const;

    virtual Address
    GetBroadcast(void) const;

    virtual bool
    IsMulticast(void) const;

    virtual Address
    GetMulticast(Ipv4Address multicastGroup) const;

    virtual bool
    IsPointToPoint(void) const;

    virtual bool
    IsBridge(void) const;

    virtual bool
    Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

    virtual bool
    SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest,
        uint16_t protocolNumber);

    virtual Ptr<Node>
    GetNode(void) const;

    virtual void
    SetNode(Ptr<Node> node);

    virtual bool
    NeedsArp(void) const;

    virtual void
    SetReceiveCallback(NetDevice::ReceiveCallback cb);

    virtual Address
    GetMulticast(Ipv6Address addr) const;

    virtual void
    SetPromiscReceiveCallback(PromiscReceiveCallback cb);

    virtual bool
    SupportsSendFrom(void) const;

    /**
     * Get the attached input queue.
     *
     * This method is provided for any derived class that may need to get
     * direct access to the underlying queue.
     *
     * \return a pointer to the queue.
     */
    Ptr<Queue>
    GetInQueue () const;

    /**
     * Dequeus a packet from the in queue.
     * The method does nothing if no in queue is defined or the in queue is empty.
     *
     * \return the dequeued packet
     */
    Ptr<const Packet>
    DequeuePacketFromInQueue ();

    /**
     * Sets the net device used as an output port for sending a message from this
     * net device. If a via net device is set, its channel will be used for
     * sending the current packet of this net device. Otherwise, this net
     * device will use its channel to make the send.
     *
     * \param viaNetDevice the via net device
     */
    void
    SetViaNetDevice (Ptr<NocNetDevice> viaNetDevice);

    /**
     * \see SetViaNetDevice (Ptr<NocNetDevice> viaNetDevice)
     * \return the vie net device
     */
    Ptr<NocNetDevice>
    GetViaNetDevice () const;

  protected:

    virtual void
    DoDispose ();

    /**
     * Get the attached output queue.
     *
     * This method is provided for any derived class that may need to get
     * direct access to the underlying queue.
     *
     * \return a pointer to the queue.
     */
    Ptr<Queue>
    GetOutQueue () const;

    /**
     * Event for processing the packet from the head of the input queue.
     * This event will reschedule itself if the queue still contains packets.
     *
     * \param originalHeader the header of the packet before routing
     * \param packet the last packet received (will be put in the queue's tail)
     */
    virtual void
    ProcessBufferedPackets (NocHeader originalHeader, Ptr<Packet> packet);

  private:

    /**
     * Device ID returned by the channel when this device is added to it
     */
    uint32_t m_deviceId;

    Ptr<NocChannel> m_channel;

    /**
     * The Queue which this NoC net device uses as a packet sink (channel input buffering).
     *
     * \see class Queue
     * \see class DropTailQueue
     */
    Ptr<Queue> m_inQueue;

    class SrcDest
    {
    private:
      Mac48Address m_src;

      Mac48Address m_dest;

      Ptr<NocNetDevice> m_viaNetDevice;

    public:

      SrcDest ()
      {
        m_src = 0;
        m_dest = 0;
        m_viaNetDevice = 0;
      }

      SrcDest (Mac48Address src, Mac48Address dest, Ptr<NocNetDevice> viaNetDevice)
      {
        m_src = src;
        m_dest = dest;
        m_viaNetDevice = viaNetDevice;
      }

      Mac48Address
      GetSrc () const
      {
        return m_src;
      }

      Mac48Address
      GetDest () const
      {
        return m_dest;
      }

      Ptr<NocNetDevice>
      GetViaNetDevice () const
      {
        return m_viaNetDevice;
      }

    };

    std::map<Ptr<const Packet>,SrcDest> m_pktSrcDestMap;

    /**
     * The Queue which this NoC net device uses as a packet source (channel output buffering).
     *
     * \see class Queue
     * \see class DropTailQueue
     */
    Ptr<Queue> m_outQueue; // TODO we don't use output channel buffering for the moment

    /**
     * Marks the head packet as blocked (at this net device).
     * The marking is made using a NocPacketTag.
     *
     * \param packet the head packet
     */
    void
    MarkHeadPacketAsBlocked (Ptr<Packet> packet);

    /**
     * Marks the head packet as unblocked (at this net device).
     * The marking is made using a NocPacketTag.
     *
     * \param packet the head packet
     */
    void
    MarkHeadPacketAsUnblocked (Ptr<Packet> packet);

    NetDevice::ReceiveCallback m_rxCallback;

    NetDevice::PromiscReceiveCallback m_promiscCallback;

    Ptr<Node> m_node;

    uint16_t m_mtu;

    uint32_t m_ifIndex;

    Mac48Address m_address;

    /**
     * Marks to what direction this NoC net device can route packets to.
     * This field should take the value from an enum (enumeration) taken
     * from each particular routing protocol.
     *
     * Value 0 (zero) should always mean no routing direction.
     */
    int m_routingDirection;

    /**
     * Marks the topological dimension in which this NoC net device routes packets.
     */
    int m_routingDimension;

    /**
     * The trace source fired when packets are sent.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet> > m_sendTrace;

    /**
     * The trace source fired when packets are received.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet> > m_receiveTrace;

    /**
     * The net device used as an output port for sending a message from this
     * net device. If a via net device is set, its channel will be used for
     * sending the current packet of this net device. Otherwise, this net
     * device will use its channel to make the send.
     */
    Ptr<NocNetDevice> m_viaNetDevice;

    /**
     * at what time was the last event scheduled (this prevents scheduling
     * an event more than once, at the same time)
     */
    Time m_lastScheduledEvent;
  };

} // namespace ns3

#endif /* NOCNETDEVICE_H_ */
