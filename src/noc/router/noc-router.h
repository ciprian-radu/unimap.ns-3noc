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
#include "ns3/route.h"
#include <vector>
#include "src/noc/orion/SIM_router.h"
#include "src/noc/orion/SIM_router_power.h"
#include "src/noc/orion/SIM_router_area.h"
#include "src/noc/orion/SIM_util.h"

namespace ns3
{

  class NocNode;
  class NocNetDevice;
  class NocRoutingProtocol;
  class NocSwitchingProtocol;
  class LoadRouterComponent;

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
     * \return the load router component, or NULL if no load component is used
     */
    virtual Ptr<LoadRouterComponent>
    GetLoadRouterComponent () const;

    /**
     * Adds the load received from a neighbor router (marked by its topological direction)
     *
     * \param load the load information received from a neighbor
     * \param sourceDevice the net device from which the load information came
     */
    virtual void
    AddNeighborLoad (int load, Ptr<NocNetDevice> sourceDevice);

    /**
     * Retrieves the load of the router which is a neighbor to this router
     * (by being connected to this router through sourceDevice)
     *
     * \param sourceDevice the net device of this router
     *
     * \return the load
     */
    virtual int
    GetNeighborLoad (Ptr<NocNetDevice> sourceDevice);

    /**
     * Retrieves the load of the router which is a neighbor to this router,
     * in the specified direction.
     *
     * \param sourceDevice the net device of this router (it marks where the packet is in this router)
     * \param direction the direction of the neighbor
     * \param dimension the topological dimension
     *
     * \return the load
     */
    virtual int
    GetNeighborLoad (Ptr<NocNetDevice> sourceDevice, int direction, int dimension);

    /**
     * Computes the occupancy of all the in channels of this router.
     * The occupancy represents how many packets are in the input channels of this router,
     * reported to the size of the queue.
     * Note that if the router has no in channels, the occupancy is zero.
     *
     * \param sourceDevice a net device belonging to this router, which determined the occupancy computation
     *                     (this parameter is optional, i.e. can be NULL)
     *
     * \return the occupancy of the in channels
     */
    virtual double
    GetInChannelsOccupancy (Ptr<NocNetDevice> sourceDevice);

    /**
     * Computes the occupancy of all the out channels of this router.
     * The occupancy represents how many packets are in the output channels of this router,
     * reported to the size of the queue.
     * Note that if the router has no out channels, the occupancy is zero.
     *
     * \param sourceDevice a net device belonging to this router, which determined the occupancy computation
     *                     (this parameter is optional, i.e. can be NULL)
     *
     * \return the occupancy of the out channels
     */
    virtual double
    GetOutChannelsOccupancy (Ptr<NocNetDevice> sourceDevice);

    /**
     * Register the routing protocol.
     */
    virtual void
    SetRoutingProtocol (Ptr<NocRoutingProtocol> routingProtocol);

    /**
     * Access current routing protocol
     */
    virtual Ptr<NocRoutingProtocol>
    GetRoutingProtocol ();

    /**
     * Register the switching protocol.
     */
    virtual void
    SetSwitchingProtocol (Ptr<NocSwitchingProtocol> switchingProtocol);

    /**
     * Access current switching protocol
     */
    virtual Ptr<NocSwitchingProtocol>
    GetSwitchingProtocol ();

    /**
     * Allows the router to manage the packet. Package management means switching and routing.
     *
     * For requesting routing information, all packets must go through this request.
     *
     *
     * \param source        source NoC net device
     * \param destination   destination address
     * \param packet        the packet to be resolved (needed the whole packet, because
     *                      routing information is added as tags or headers). The packet
     *                      will be returned to reply callback.
     * \param protocolType  protocol ID, needed to form a proper MAC-layer header
     *
     * \return the route, if a valid route is already known, NULL otherwise
     */
    virtual Ptr<Route>
    ManagePacket (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet);

    /**
     * \param packet the packet to be sent
     * \param destination the destination node of this packet
     *
     * \return the net device which can be used to inject the packet into the network
     */
    virtual Ptr<NocNetDevice>
    GetInjectionNetDevice (Ptr<Packet> packet, Ptr<NocNode> destination) = 0;

    virtual std::vector<Ptr<NocNetDevice> >
    GetInjectionNetDevices () = 0;

    /**
     *
     * \return the net device which can be used to receive the packet,
     *         from the network, for the processing element (NoC application)
     */
    virtual Ptr<NocNetDevice>
    GetReceiveNetDevice () = 0;

    /**
     * Searches for the NoC net device which must be used by the destination node to receive the packet.
     * The search is based on the NoC net device through which the packet will come, and also the direction
     * through which the packet is sent (determined by the routing protocol).
     *
     * \param sender the sender NoC net device
     * \param routingDirection in what direction the packet will go (routing protocol dependent)
     * \param routingDimension in what topological dimension the routing direction applies
     *
     * \return the input net device
     */
    virtual Ptr<NocNetDevice>
    GetInputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension) = 0;

    /**
     * Searches for the NoC net device which must be used to forward the message to the next node.
     * The search is based on the NoC net device through which the packet came, and also the direction
     * through which the packet is sent (determined by the routing protocol).
     *
     * \param sender the sender NoC net device
     * \param routingDirection in what direction the packet will go (routing protocol dependent)
     * \param routingDimension in what topological dimension the routing direction applies
     *
     * \return the output net device
     */
    virtual Ptr<NocNetDevice>
    GetOutputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension) = 0;

    /**
     * Retrieves all the possible output net devices for a packet sent by the specified net device
     *
     * \param packet the packet
     * \param sender the net device which sent the packet
     *
     * \return an array with the output net devices
     */
    virtual std::vector<Ptr<NocNetDevice> >
    GetOutputNetDevices (Ptr<Packet> packet, Ptr<NocNetDevice> sender) = 0;

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
    virtual void
    SetNocNode (Ptr<NocNode> nocNode);

    /**
     * \return the NoC node to which this routing protocol is assigned to
     */
    virtual Ptr<NocNode>
    GetNocNode () const;

    /**
     * \return the name of this routing protocol
     */
    virtual std::string
    GetName () const;

  private:

    /**
     * This function is just a copy of the SIM_router_init function from ORION (SIM_router.cc).
     * We use this approach because we want to be able to set some router parameters dynamically
     * (the PARM approach is not very useful).
     */
    int
    routerInitForOrion (SIM_router_info_t *info, SIM_router_power_t *router_power, SIM_router_area_t *router_area);

  public:

    /**
     * \return how many input ports the router has
     */
    virtual uint32_t
    GetNumberOfInputPorts () = 0;

    /**
     * \return how many output ports the router has
     */
    virtual uint32_t
    GetNumberOfOutputPorts () = 0;

    /**
     * \return how many virtual channels the router has
     */
    virtual uint32_t
    GetNumberOfVirtualChannels () = 0;

    /**
     * Uses ORION to measure this router's area.
     *
     * \return the area, in um^2
     */
    virtual double
    GetArea ();

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

    /**
     * stores the injection net device for each head packet encountered
     */
    std::map<uint32_t, Ptr<NocNetDevice> > m_headPacketsInjectionNetDevice;

  private:

    /**
     * the name of the routing protocol
     */
    std::string m_name;

  };

} // namespace ns3

#endif /* NOCROUTER_H_ */
