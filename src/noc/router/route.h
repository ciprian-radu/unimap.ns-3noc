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

#ifndef ROUTE_H_
#define ROUTE_H_

#include "ns3/object.h"
#include "ns3/noc-header.h"
#include "ns3/packet.h"
#include "ns3/noc-node.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  /**
   *
   * \brief Holds the route for a packet, as it is determined by the routing protocol.
   *
   */
  class Route : public Object
  {
  public:

    static TypeId
    GetTypeId ();

    Route (Ptr<Packet> routedPacket, Ptr<NocNetDevice> sourceDevice, Ptr<NocNetDevice> destinationDevice);

    ~Route ();

    Ptr<Packet>
    GetRoutedPacket () const;

    Ptr<NocNetDevice>
    GetSourceDevice () const;

    Ptr<NocNetDevice>
    GetDestinationDevice () const;

  protected:

  private:

    /**
     * the routed packet packet
     */
    Ptr<Packet> m_routedPacket;

    /**
     * the net device which sends the packet
     */
    Ptr<NocNetDevice> m_sourceDevice;

    /**
     * the net device which receives the packet
     */
    Ptr<NocNetDevice> m_destinationDevice;

  };

} // namespace ns3

#endif /* ROUTE_H_ */
