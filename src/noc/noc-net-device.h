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
#include "noc-channel.h"
#include "ns3/traced-callback.h"
#include "noc-routing-protocol.h"
#include "noc-helper.h"

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

    void
    Receive(Ptr<Packet> packet, uint16_t protocol, Mac48Address to, Mac48Address from);

    void
    SetChannel(Ptr<NocChannel> channel);

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

  private:
    Ptr<NocChannel> m_channel;

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
