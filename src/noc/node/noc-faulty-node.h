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

#ifndef NOCFAULTYNODE_H_
#define NOCFAULTYNODE_H_

#include "ns3/noc-node.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-net-device.h"
#include "ns3/noc-router.h"

namespace ns3
{

  /**
   * A NocNode which can be faulty.
   * The faulty property is an ns-3 attribute.
   *
   * \see NocNode
   */
  class NocFaultyNode : public NocNode
  {
  public:

    static TypeId
    GetTypeId ();

    NocFaultyNode ();

    virtual
    ~NocFaultyNode ();

    ///\name Routers
    //\{
    /// Register the router.
    virtual void
    SetRouter (Ptr<NocRouter> router);

    /// Access current router
    virtual Ptr<NocRouter>
    GetRouter ();
    //\}

//    void
//    SetFaulty (bool faulty);
//
//    bool
//    IsFaulty ();

    /**
     * Injects a specified packet into the NoC. The packet will be sent to the specified destination node.
     *
     * \param packet the network packet
     * \param destination the destination node
     */
    virtual void
    InjectPacket (Ptr<NocPacket> packet, Ptr<NocNode> destination);

    /**
     * Sends a packet through the network, from the specified NoC net device
     *
     * \param source the source noC net device
     * \param packet the network packet
     * \param destination the destination node
     */
    virtual void
    Send (Ptr<NocNetDevice> source, Ptr<Packet> packet, Ptr<NocNode> destination);

  private:

    /**
     * whether or not the node is faulty
     */
    bool m_faulty;

  };

} // namespace ns3

#endif /* NOCFAULTYNODE_H_ */
