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

#ifndef IRVINEROUTER_H_
#define IRVINEROUTER_H_

#include "ns3/noc-router.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  /**
   * \brief Implementation of the routing mechanism from this paper:
   *
   * Increasing the throughput of an adaptive router in network-on-chip (NoC)
   *
   * http://portal.acm.org/citation.cfm?id=1176276
   */
  class IrvineRouter : public NocRouter
  {
  public:

    static TypeId
    GetTypeId ();

    IrvineRouter ();

    virtual
    ~IrvineRouter ();

    virtual Ptr<NocNetDevice>
    GetInjectionNetDevice (Ptr<Packet> packet, Ptr<NocNode> destination);

    virtual std::vector<Ptr<NocNetDevice> >
    GetInjectionNetDevices ();

    virtual Ptr<NocNetDevice>
    GetReceiveNetDevice ();

    virtual uint32_t
    AddDevice (Ptr<NocNetDevice> device);

    /**
     * Retrieves all the possible output net devices for a packet sent by the specified net device.
     * Note that once a packet reaches the column where the destination is (X distance is zero),
     * East and West outputs will not be returned, because the Irvine router doesn't allow turning
     * from East to West or vice versa.
     *
     * \param packet the packet
     * \param sender the net device which sent the packet
     *
     * \return an array with the output net devices
     */
    std::vector<Ptr<NocNetDevice> >
    GetOutputNetDevices (Ptr<Packet> packet, Ptr<NocNetDevice> sender);

    /**
     * Determines if the given net device belongs to the left router
     *
     * \param sender the net device
     *
     * \return whether or not the net device belongs to the left router
     */
    bool
    isLeftRouter (Ptr<NocNetDevice> sender);

    /**
     * Determines if the given net device belongs to the right router
     *
     * \param sender the net device
     *
     * \return whether or not the net device belongs to the right router
     */
    bool
    isRightRouter (Ptr<NocNetDevice> sender);

    /**
     * Computes the occupancy of the in channels of this router, either left or right router component (see below).
     * The occupancy represents how many packets are in the input channels of this router,
     * reported to the size of the queue.
     * Note that if the router has no in channels, the occupancy is zero.
     *
     * \param sourceDevice a net device belonging to this router, which determined the occupancy computation
     *                     (cannot be NULL; this tells if either left or right router channels should be considered)
     *
     * \return the occupancy of the in channels
     */
    double
    GetInChannelsOccupancy (Ptr<NocNetDevice> sourceDevice);

    /**
     * Computes the occupancy of the out channels of this router, either left or right router component (see below).
     * The occupancy represents how many packets are in the output channels of this router,
     * reported to the size of the queue.
     * Note that if the router has no out channels, the occupancy is zero.
     *
     * \param sourceDevice a net device belonging to this router, which determined the occupancy computation
     *                     (cannot be NULL; this tells if either left or right router channels should be considered)
     *
     * \return the occupancy of the out channels
     */
    double
    GetOutChannelsOccupancy (Ptr<NocNetDevice> sourceDevice);

    virtual void
    SetNocNode (Ptr<NocNode> nocNode);

    /**
     * \return how many input ports the router has
     */
    virtual uint32_t
    GetNumberOfInputPorts ();

    /**
     * \return how many output ports the router has
     */
    virtual uint32_t
    GetNumberOfOutputPorts ();

    /**
     * \return how many virtual channels the router has
     */
    virtual uint32_t
    GetNumberOfVirtualChannels ();

  protected:

    IrvineRouter (std::string name);

    Ptr<NocNetDevice>
    GetInputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension);

    Ptr<NocNetDevice>
    GetOutputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension);

  private:

    /**
     * Initialize the router
     */
    void
    Init ();

    Ptr<NocNetDevice> m_internalLeftInputDevice;

    Ptr<NocNetDevice> m_internalRightInputDevice;

    Ptr<NocNetDevice> m_internalOutputDevice;

    /**
     * the input net devices associated to the left router (subset of m_devices)
     */
    std::vector<Ptr<NocNetDevice> > m_leftRouterInputDevices;

    /**
     * the output net devices associated to the left router (subset of m_devices)
     */
    std::vector<Ptr<NocNetDevice> > m_leftRouterOutputDevices;

    /**
     * the input net devices associated to the right router (subset of m_devices)
     */
    std::vector<Ptr<NocNetDevice> > m_rightRouterInputDevices;

    /**
     * the output net devices associated to the right router (subset of m_devices)
     */
    std::vector<Ptr<NocNetDevice> > m_rightRouterOutputDevices;

    bool m_north1DeviceAdded;

    bool m_north2DeviceAdded;

    bool m_eastDeviceAdded;

    bool m_south1DeviceAdded;

    bool m_south2DeviceAdded;

    bool m_westDeviceAdded;

  };

} // namespace ns3

#endif /* IRVINEROUTER_H_ */
