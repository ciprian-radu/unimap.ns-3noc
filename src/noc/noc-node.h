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
#include "noc-packet.h"
#include "noc-net-device.h"
#include "noc-routing-protocol.h"

namespace ns3
{

  class NocRoutingProtocol;
  class NocNetDevice;

  class NocNode : public Node
  {
  public:

    static TypeId
    GetTypeId();

    NocNode();

    NocNode(uint32_t systemId);

    virtual
    ~NocNode();

    ///\name Protocols
    //\{
    /// Register routing protocol.
    void
    SetRoutingProtocol(Ptr<NocRoutingProtocol> protocol);

    /// Access current routing protocol
    Ptr<NocRoutingProtocol>
    GetRoutingProtocol();
    //\}

    void
    Send (Ptr<Packet>, Ptr<NocNode>);

    void
    DoSend (Ptr<Packet>, Ptr<NetDevice>, Ptr<NetDevice>);

  private:

    /**
     * The routing protocol
     */
    Ptr<NocRoutingProtocol> m_routingProtocol;

  };

} // namespace ns3

#endif /* NOCNODE_H_ */
