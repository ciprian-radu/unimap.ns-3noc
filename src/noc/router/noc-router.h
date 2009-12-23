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
#include "ns3/noc-switching-protocol.h"
#include "ns3/load-router-component.h"
#include <vector>

namespace ns3
{

  class NocNode;
  class NocNetDevice;
  class NocRoutingProtocol;
  class NocSwitchingProtocol;

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

    NocRouter (std::string name);

    NocRouter (std::string name, Ptr<LoadRouterComponent> loadComponent);

    virtual
    ~NocRouter ();

    /**
     * Register the routing protocol.
     */
    void
    SetRoutingProtocol (Ptr<NocRoutingProtocol> routingProtocol);

    /**
     * Access current routing protocol
     */
    Ptr<NocRoutingProtocol>
    GetRoutingProtocol ();

    /**
     * Register the switching protocol.
     */
    void
    SetSwitchingProtocol (Ptr<NocSwitchingProtocol> switchingProtocol);

    /**
     * Access current switching protocol
     */
    Ptr<NocSwitchingProtocol>
    GetSwitchingProtocol ();

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
     * Allows the router to manage the packet. Package management means switching and routing.
     *
     * For requesting routing information, all packets must go through this request.
     *
     * Note that route discovery works async. -- RequestRoute returns immediately, while
     * reply callback will be called when routing information will be available.
     *
     * \param source        source NoC net device
     * \param destination   destination address
     * \param packet        the packet to be resolved (needed the whole packet, because
     *                      routing information is added as tags or headers). The packet
     *                      will be returned to reply callback.
     * \param protocolType  protocol ID, needed to form a proper MAC-layer header
     * \param routeReply    callback to be invoked after route discovery procedure, supposed
     *                      to really send packet using routing information.
     *
     * \return true if a valid route is already known
     */
    virtual bool
    ManagePacket(const Ptr<NocNetDevice> source,
        const Ptr<NocNode> destination, Ptr<Packet> packet, RouteReplyCallback routeReply);

    virtual Ptr<NocNetDevice>
    GetInjectionNetDevice (Ptr<NocPacket> packet, Ptr<NocNode> destination) = 0;

    /**
     * Searches for the NoC net device which must be used by the destination node to receive the packet.
     * The search is based on the NoC net device through which the packet will come, and also the direction
     * through which the packet is sent (determined by the routing protocol).
     *
     * \param sender the sender NoC net device
     * \param routingDirection in what direction the packet will go (routing protocol dependent)
     *
     * \return the input net device
     */
    virtual Ptr<NocNetDevice>
    GetInputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection) = 0;

    /**
     * Searches for the NoC net device which must be used to forward the message to the next node.
     * The search is based on the NoC net device through which the packet came, and also the direction
     * through which the packet is sent (determined by the routing protocol).
     *
     * \param sender the sender NoC net device
     * \param routingDirection in what direction the packet will go (routing protocol dependent)
     *
     * \return the output net device
     */
    virtual Ptr<NocNetDevice>
    GetOutputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection) = 0;

    /**
     * Retrieves all the possible output net devices for a packet sent by the specified net device
     *
     * \param sender the net device which sent the packet
     *
     * \return an array with the output net devices
     */
    virtual std::vector<Ptr<NocNetDevice> >
    GetOutputNetDevices (Ptr<NocNetDevice> sender) = 0;

    /**
     * Associate this device to this router.
     *
     * \param device NetDevice to associate to this router.
     *
     * \return the index of the NetDevice into the router's list of
     *         net devices.
     */
    virtual uint32_t
    AddDevice (Ptr<NocNetDevice> device);

    /**
     * The indexes used by the GetDevice method start at one and
     * end at GetNDevices ()
     *
     * \param index the index of the requested NetDevice
     *
     * \return the requested NetDevice associated to this router.
     *
     */
    virtual Ptr<NocNetDevice>
    GetDevice (uint32_t index) const;

    /**
     * \return the number of NetDevice instances associated
     *         to this router.
     */
    virtual uint32_t
    GetNDevices (void) const;

    /**
     * set the NoC node to which this routing protocol is assigned to
     */
    void
    SetNocNode (Ptr<NocNode> nocNode);

    /**
     * \return the NoC node to which this routing protocol is assigned to
     */
    Ptr<NocNode>
    GetNocNode () const;

    /**
     * \return the name of this routing protocol
     */
    std::string
    GetName () const;

  protected:

    /**
     * the NoC node to which this router is assigned to
     */
    Ptr<NocNode> m_nocNode;

    /**
     * the routing protocol assigned to this router
     */
    Ptr<NocRoutingProtocol> m_routingProtocol;

    /**
     * the switching protocol assigned to this router
     */
    Ptr<NocSwitchingProtocol> m_switchingProtocol;

    std::vector<Ptr<NocNetDevice> > m_devices;

    /**
     * the load router component
     */
    Ptr<LoadRouterComponent> m_loadComponent;

  private:

    /**
     * the name of the routing protocol
     */
    std::string m_name;

  };

} // namespace ns3

#endif /* NOCROUTER_H_ */
