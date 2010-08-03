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

#ifndef NOCNETDEVICE_H_
#define NOCNETDEVICE_H_

#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include <stdint.h>
#include <string>
#include "ns3/noc-channel.h"
#include "ns3/traced-callback.h"
#include "ns3/noc-routing-protocol.h"
#include "ns3/noc-helper.h"
#include "ns3/queue.h"
#include <map>

namespace ns3
{

  class NocChannel;
  class NocRoutingProtocol;
  class NocHelper;

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

    void
    Receive(Ptr<Packet> packet, Mac48Address to, Mac48Address from);

    void
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
    void SetInQueue (Ptr<Queue> inQueue);

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
    void SetOutQueue (Ptr<Queue> outQueue);

    /**
     * This method should be invoked whenever a packet is dropped by the net device
     *
     * \param packet the dropped packet
     */
    void
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
    SetRoutingDirection (int routingDirection);

    virtual int
    GetRoutingDirection () const;

    virtual void
    SetNocHelper (Ptr<NocHelper> nocHelper);

    virtual Ptr<NocHelper>
    GetNocHelper () const;

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

  protected:

    virtual void
    DoDispose(void);

    /**
     * Get the attached input queue.
     *
     * This method is provided for any derived class that may need to get
     * direct access to the underlying queue.
     *
     * \return a pointer to the queue.
     */
    Ptr<Queue> GetInQueue () const;

    /**
     * Get the attached output queue.
     *
     * This method is provided for any derived class that may need to get
     * direct access to the underlying queue.
     *
     * \return a pointer to the queue.
     */
    Ptr<Queue> GetOutQueue () const;

    /**
     * Event for processing the packet from the head of the input queue.
     * This event will reschedule itself if the queue still contains packets.
     *
     * \param packet the last packet received (will be put in the queue's tail)
     */
    virtual void
    ProcessBufferedPackets (Ptr<Packet> packet);

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

    public:

      SrcDest ()
      {
        m_src = 0;
        m_dest = 0;
      }

      SrcDest (Mac48Address src, Mac48Address dest)
      {
        m_src = src;
        m_dest = dest;
      }

      Mac48Address
      GetSrc() const
      {
        return m_src;
      }

      Mac48Address
      GetDest() const
      {
        return m_dest;
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
    markHeadPacketAsBlocked (Ptr<Packet> packet);

    /**
     * Marks the head packet as unblocked (at this net device).
     * The marking is made using a NocPacketTag.
     *
     * \param packet the head packet
     */
    void
    markHeadPacketAsUnblocked (Ptr<Packet> packet);

    NetDevice::ReceiveCallback m_rxCallback;

    NetDevice::PromiscReceiveCallback m_promiscCallback;

    Ptr<Node> m_node;

    uint16_t m_mtu;

    uint32_t m_ifIndex;

    Mac48Address m_address;

    /**
     * The NoC topology
     */
    Ptr<NocHelper> m_nocHelper;

    /**
     * Marks to what direction this NoC net device can route packets to.
     * This field should take the value from an enum (enumeration) taken
     * from each particular routing protocol.
     *
     * Value 0 (zero) should always mean no routing direction.
     */
    int m_routingDirection;

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
  };

} // namespace ns3

#endif /* NOCNETDEVICE_H_ */
