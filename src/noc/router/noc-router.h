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

#ifndef NOCROUTER_H_
#define NOCROUTER_H_

#include "ns3/object.h"
#include "ns3/mac48-address.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-node.h"
#include "ns3/noc-net-device.h"
#include "ns3/noc-routing-protocol.h"

namespace ns3
{

  class NocNode;
  class NocNetDevice;
  class NocRoutingProtocol;

  /**
   *
   * \brief Interface for the NoC router
   *
   * Every NoC router must implement this interface. Each NoC router is supposed
   * to know of a single routing protocol to work with, see NocNode::SetRoutingProtocol ().
   *
   */
  class NocRouter : public Object
  {
  public:

    static TypeId
    GetTypeId();

    NocRouter(std::string name);

    virtual
    ~NocRouter();

    ///\name Routers
    //\{
    /// Register the routing protocol.
    void
    SetRoutingProtocol (Ptr<NocRoutingProtocol> router);

    /// Access current routing protocol
    Ptr<NocRoutingProtocol>
    GetRoutingProtocol ();
    //\}

    /**
     * Callback to be invoked when the route discovery procedure is completed.
     *
     * \param flag        indicating whether a route was actually found and all needed information is
     *                    added to the packet successfully
     *
     * \param packet      the NoC packet for which the route was resolved
     *
     * \param src         source address of the packet
     *
     * \param dst         destination address of the packet
     *
     * \param protocol    ethernet 'Protocol' field, needed to form a proper MAC-layer header
     *
     */
    typedef Callback<void,/* return type */
    Ptr<Packet> , /* packet */
    Ptr<NetDevice>,/* src */
    Ptr<NetDevice>/* dst */
    > RouteReplyCallback;

    /**
     * Request routing information, all packets must go through this request.
     *
     * Note that route discovery works async. -- RequestRoute returns immediately, while
     * reply callback will be called when routing information will be available.
     *
     * \return true if a valid route is already known
     * \param source        source address
     * \param destination   destination address
     * \param packet        the packet to be resolved (needed the whole packet, because
     *                      routing information is added as tags or headers). The packet
     *                      will be returned to reply callback.
     * \param protocolType  protocol ID, needed to form a proper MAC-layer header
     * \param routeReply    callback to be invoked after route discovery procedure, supposed
     *                      to really send packet using routing information.
     */
    virtual bool
    RequestRoute(const Ptr<NocNode> source,
        const Ptr<NocNode> destination, Ptr<Packet> packet, RouteReplyCallback routeReply) = 0;

    /**
     * set the NoC node to which this routing protocol is assigned to
     */
    void
    SetNocNode(Ptr<NocNode> nocNode);

    /**
     * \return the NoC node to which this routing protocol is assigned to
     */
    Ptr<NocNode>
    GetNocNode() const;

    /**
     * \return the name of this routing protocol
     */
    std::string
    GetName() const;

  protected:
    /**
     * the NoC node to which this router is assigned to
     */
    Ptr<NocNode> m_nocNode;

    /**
     * the routing protocol assigned to this router
     */
    Ptr<NocRoutingProtocol> m_routingProtocol;

  private:
    /**
     * the name of the routing protocol
     */
    std::string m_name;
  };

} // namespace ns3

#endif /* NOCROUTER_H_ */
