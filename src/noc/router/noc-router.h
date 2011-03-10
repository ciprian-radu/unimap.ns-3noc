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
#include "src/noc/orion/SIM_clock.h"

using namespace std;

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

    NocRouter (string name);

    NocRouter (string name, Ptr<LoadRouterComponent> loadComponent);

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
     * Allows the router to manage the flit.
     * This implies the flit enters the router and traverses the pipeline stages (e.g.: routing, switching).
     *
     * \param source        source NoC net device
     * \param destination   destination address
     * \param flit          the flit to be resolved (needed the whole flit, because
     *                      routing information is added as tags or headers). The flit
     *                      will be returned to reply callback.
     *
     * \return the route, if a valid route is already known, NULL otherwise
     */
    virtual Ptr<Route>
    ManageFlit (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> flit);

    /**
     * \param packet the packet to be sent
     * \param destination the destination node of this packet
     *
     * \return the net device which can be used to inject the packet into the network
     */
    virtual Ptr<NocNetDevice>
    GetInjectionNetDevice (Ptr<Packet> packet, Ptr<NocNode> destination) = 0;

    virtual vector<Ptr<NocNetDevice> >
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
    virtual vector<Ptr<NocNetDevice> >
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
    virtual string
    GetName () const;

  private:

    /**
     * This function is just a copy of the SIM_router_init function from ORION (SIM_router.cc).
     * We use this approach because we want to be able to set some router parameters dynamically
     * (the PARM approach is not very useful).
     */
    int
    RouterInitForOrion (SIM_router_info_t *info, SIM_router_power_t *router_power, SIM_router_area_t *router_area);

    /*
     * This is a replica of ORION's SIM_router_stat_energy (...) function.
     * We modified the original function so that we can avoid printing the results and grab them for usage.
     *
     * time unit:   1 cycle
     * e_fin:       average # of flits received by one input port during unit time
     *                (at most 0.5 for InfiniBand router)
     * e_buf_wrt:   average # of input buffer writes of all ports during unit time
     *              e_buf_wrt = e_fin * n_buf_in
     * e_buf_rd:    average # of input buffer reads of all ports during unit time
     *              e_buf_rd = e_buf_wrt
     *                (splitted into different input ports in program)
     * e_cbuf_fin:  average # of flits passing through the switch during unit time
     *              e_cbuf_fin = e_fin * n_total_in
     * e_cbuf_wrt:  average # of central buffer writes during unit time
     *              e_cbuf_wrt = e_cbuf_fin / (pipe_depth * pipe_width)
     * e_cbuf_rd:   average # of central buffer reads during unit time
     *              e_cbuf_rd = e_cbuf_wrt
     * e_arb:       average # of arbitrations per arbiter during unit time
     *              assume e_arb = 1
     *
     * NOTES: (1) negative print_depth means infinite print depth
     *
     */
    virtual double
    ComputeRouterEnergyAndPowerWithOrion (SIM_router_info_t *info, SIM_router_power_t *router, int print_depth, char *path, int max_avg, double e_fin, int plot_flag, double freq);

    /**
     * Uses ORION to measure this router's power and energy, consumed for sending the specified flit.
     *
     * \param flit the flit
     *
     */
    virtual void
    MeasurePowerAndEnergy (Ptr<Packet> flit);

    /**
     * Power and energy are measured for the previous clock cycle.
     * This method is invoked when method GetDynamicPower (), GetLeakagePower () or GetTotalPower () are called,
     * with the purpose to measure power and energy for the last clock too.
     * The method has effect only if there are flits for which power wasn't measured.
     */
    void
    MeasurePowerAndEnergyForLastClock ();

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
     * Uses ORION to get the dynamic power consumed by this router.
     *
     * \return the dynamic power, in Watt
     */
    virtual double
    GetDynamicPower ();

    /**
     * Uses ORION to get the leakage power consumed by this router.
     *
     * \return the dynamic power, in Watt
     */
    virtual double
    GetLeakagePower ();

    /**
     * \see GetDynamicPower
     * \see GetLeakagePower
     *
     * \return the dynamic + leakage power, in Watt
     */
    virtual double
    GetTotalPower ();

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

    vector<Ptr<NocNetDevice> > m_devices;

    /**
     * the load router component
     */
    Ptr<LoadRouterComponent> m_loadComponent;

    /**
     * stores the injection net device for each head packet encountered
     */
    map<uint32_t, Ptr<NocNetDevice> > m_headPacketsInjectionNetDevice;

  private:

    /**
     * the name of the routing protocol
     */
    string m_name;

    /**
     * how many times power consumption was measured
     */
    uint64_t m_powerCounter;

    /**
     * the last clock when power consumption was measured
     */
    uint64_t m_lastClock;

    /**
     * the flits from the previous clock cycle, for which power will have to be measured
     */
    vector<Ptr<Packet> > m_flitsFromLastClock;

    /**
     * the number of flits arrived at the router's all input ports, only during one clock cycle
     */
    uint64_t m_arrivedFlits;

    /**
     * the dynamic power consumed during all clock cycles
     * (divide this with m_powerCounter to get the (average) dynamic power consumed by this router)
     */
    double m_dynamicPower;

    /**
     * the leakage power consumed during all clock cycles
     * (divide this with m_powerCounter to get the (average) leakage power consumed by this router)
     */
    double m_leakagePower;

  };

} // namespace ns3

#endif /* NOCROUTER_H_ */
