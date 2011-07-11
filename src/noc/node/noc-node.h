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

#ifndef NOCNODE_H_
#define NOCNODE_H_

#include "ns3/node.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-net-device.h"
#include "ns3/noc-router.h"
#include <vector>
#include <map>

namespace ns3
{

  class NocRouter;
  class NocNetDevice;

  /**
   *
   * The default implementation for a Network-on-Chip node.
   * All NoC node classes should be derived from this class.
   *
   */
  class NocNode : public Node
  {
  public:

    static TypeId
    GetTypeId();

    NocNode();

    NocNode(uint32_t systemId);

    virtual
    ~NocNode();

    ///\name Routers
    //\{
    /// Register the router.
    virtual void
    SetRouter (Ptr<NocRouter> router);

    /// Access current router
    virtual Ptr<NocRouter>
    GetRouter ();
    //\}

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
     * \param packet the network packet
     * \param source the source noC net device
     * \param viaNetDevice the via net device (can be NULL)
     * \param destination the destination node
     */
    virtual void
    Send (Ptr<NocNetDevice> source, Ptr<Packet> packet, Ptr<NocNode> destination);

  private:

    virtual void
    DoSend (Ptr<Packet>packet, Ptr<NocNetDevice> source, Ptr<NocNetDevice> viaNetDevice, Ptr<NetDevice> destination);

    /**
     * The routing protocol
     */
    Ptr<NocRouter> m_router;

  };

} // namespace ns3

#endif /* NOCNODE_H_ */
