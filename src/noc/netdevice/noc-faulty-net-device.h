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

#ifndef NOCFAULTYNETDEVICE_H_
#define NOCFAULTYNETDEVICE_H_

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

namespace ns3
{

  /**
   * A NocNetDevice which can be faulty.
   * The faulty property is an ns-3 attribute.
   *
   * \see NocNetDevice
   */
  class NocFaultyNetDevice : public NocNetDevice
  {
  public:

    static TypeId
    GetTypeId(void);

    NocFaultyNetDevice();

    virtual
    ~NocFaultyNetDevice();

    virtual void
    Receive (Ptr<Packet> packet, Mac48Address to, Mac48Address from);

    virtual void
    SetChannel (Ptr<NocChannel> channel);

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
     * This method should be invoked whenever a packet is dropped by the net device
     *
     * \param packet the dropped packet
     */
    virtual void
    Drop (Ptr<Packet> packet);

    virtual void
    SetRoutingDirection (int routingDirection, uint32_t dimension);

    virtual int
    GetRoutingDirection () const;

    virtual int
    GetRoutingDimension () const;

    virtual bool
    Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

    virtual bool
    SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest,
        uint16_t protocolNumber);

    virtual Ptr<Node>
    GetNode () const;

    virtual void
    SetNode (Ptr<Node> node);

  protected:

  private:

    /**
     * whether or not the node is faulty
     */
    bool m_faulty;

  };

} // namespace ns3

#endif /* NOCFAULTYNETDEVICE_H_ */
