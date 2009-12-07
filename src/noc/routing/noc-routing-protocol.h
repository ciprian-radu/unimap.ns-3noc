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

#ifndef NOCROUTINGPROTOCOL_H_
#define NOCROUTINGPROTOCOL_H_

#include "ns3/object.h"
#include "ns3/mac48-address.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-node.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  class NocNode;
  class NocNetDevice;

  /**
   *
   * \brief Interface for the routing protocol used by NoC net devices
   *
   * Every routing protocol for NoCs must implement this interface. Each NoC node is supposed
   * to know of a single routing protocol to work with, see NocNode::SetRoutingProtocol ().
   *
   */
  class NocRoutingProtocol : public Object
  {
  public:

    static TypeId
    GetTypeId();

    NocRoutingProtocol(std::string name);

    virtual
    ~NocRoutingProtocol();

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
     * \param source        source NoC net device
     * \param destination   destination NoC node
     * \param packet        the packet to be resolved (needed the whole packet, because
     *                      routing information is added as tags or headers). The packet
     *                      will be returned to reply callback.
     * \param routeReply    callback to be invoked after route discovery procedure, supposed
     *                      to really send packet using routing information.
     */
    virtual bool
    RequestRoute(const Ptr<NocNetDevice> source,
        const Ptr<NocNode> destination, Ptr<Packet> packet, RouteReplyCallback routeReply);

    /**
     * \return the name of this routing protocol
     */
    std::string
    GetName() const;

  protected:

    /**
     * Whenever we encounter a new head packet we call this method. Otherwise, for data packets,
     * we use the same route that we used for the previous head packet.
     *
     * \return true if a valid route is already known
     * \param source        source NoC net device
     * \param destination   destination NoC node
     * \param packet        the packet to be resolved (needed the whole packet, because
     *                      routing information is added as tags or headers). The packet
     *                      will be returned to reply callback.
     * \param routeReply    callback to be invoked after route discovery procedure, supposed
     *                      to really send packet using routing information.
     */
    virtual bool
    RequestNewRoute(const Ptr<NocNetDevice> source,
        const Ptr<NocNode> destination, Ptr<Packet> packet, RouteReplyCallback routeReply) = 0;

    /**
     * the net device which is the source of the transmission
     * (as determined by the specific implementation of the routing protocol)
     */
    Ptr<NocNetDevice> m_sourceNetDevice;

    /**
     * the net device which is the destination of the transmission
     * (as determined by the specific implementation of the routing protocol)
     */
    Ptr<NocNetDevice> m_destinationNetDevice;

    /**
     * the number of data packets routed since the last head packet was routed
     */
    uint32_t m_dataPacketsRouted;

    /**
     * the number of data packets which must be routed
     * (determined from the header of the last head packet)
     */
    uint32_t m_dataPacketsToBeRouted;

  private:

    /**
     * the name of the routing protocol
     */
    std::string m_name;
  };

} // namespace ns3

#endif /* NOCROUTINGPROTOCOL_H_ */
