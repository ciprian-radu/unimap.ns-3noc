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
    GetInjectionNetDevice (Ptr<NocPacket> packet, Ptr<NocNode> destination);

    virtual uint32_t
    AddDevice (Ptr<NocNetDevice> device);

    std::vector<Ptr<NocNetDevice> >
    GetOutputNetDevices (Ptr<NocNetDevice> sender);

  protected:

    IrvineRouter (std::string name);

  private:

    Ptr<NocNetDevice>
    GetInputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection);

    Ptr<NocNetDevice>
    GetOutputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection);

    bool
    isRightRouter (Ptr<NocNetDevice> sender);

    bool
    isLeftRouter (Ptr<NocNetDevice> sender);

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
