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

#ifndef FOURWAYROUTER_H_
#define FOURWAYROUTER_H_

#include "ns3/noc-router.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  class FourWayRouter : public NocRouter
  {
  public:

    enum Direction {NONE, NORTH, EAST, SOUTH, WEST};

    static TypeId
    GetTypeId();

    FourWayRouter();

    virtual
    ~FourWayRouter();

    virtual Ptr<NocNetDevice>
    GetInjectionNetDevice (Ptr<Packet> packet, Ptr<NocNode> destination);

    virtual std::vector<Ptr<NocNetDevice> >
    GetInjectionNetDevices ();

    virtual Ptr<NocNetDevice>
    GetReceiveNetDevice ();

    /**
     * Retrieves all the possible output net devices for a packet sent by the specified net device.
     *
     * \param packet the packet
     * \param sender the net device which sent the packet
     *
     * \return an array with the output net devices
     */
    std::vector<Ptr<NocNetDevice> >
    GetOutputNetDevices (Ptr<Packet> packet, Ptr<NocNetDevice> sender);

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

    FourWayRouter (std::string name);

    Ptr<NocNetDevice>
    GetInputNetDevice(Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension);

    Ptr<NocNetDevice>
    GetOutputNetDevice(Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension);

  private:

    /**
     * Initialize the router
     */
    void
    Init ();

    Ptr<NocNetDevice> m_internalInputDevice;

    Ptr<NocNetDevice> m_internalOutputDevice;

  };

} // namespace ns3

#endif /* FOURWAYROUTER_H_ */
