/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *               						Lucian Blaga University of Sibiu, Romania
 *               - Systems and Networking, University of Augsburg, Germany
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
 * Author: Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *         http://webspace.ulbsibiu.ro/ciprian.radu/
 */

#ifndef NOCROUTINGPROTOCOL_H_
#define NOCROUTINGPROTOCOL_H_

#include "ns3/object.h"
#include "ns3/mac48-address.h"
#include "ns3/noc-node.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-net-device.h"
#include "ns3/route.h"
#include <map>

namespace ns3
{

  class NocNode;
  class NocNetDevice;
  class Route;

  /**
   *
   * \brief Interface for the routing protocol used by NoC net devices
   *
   * Every routing protocol for NoCs must implement this interface.
   *
   */
  class NocRoutingProtocol : public Object
  {
  public:

    /**
     * routing direction, in a particular dimension. For example, a routing protocol for a 2D mesh
     * will use two such enums: one for the horizontal axis and the other for the vertical axis
     **/
    enum RoutingDirection {NONE, FORWARD, BACK};

    static RoutingDirection
    GetOpositeRoutingDirection (int direction);

    static TypeId
    GetTypeId();

    NocRoutingProtocol(std::string name);

    virtual
    ~NocRoutingProtocol();

    /**
     * Request routing information, all packets must go through this request.
     *
     *
     * \return the Route or NULL if no route was found
     * \param source        source NoC net device
     * \param destination   destination NoC node
     * \param packet        the packet to be resolved (needed the whole packet, because
     *                      routing information is added as tags or headers)
     */
    virtual Ptr<Route>
    RequestRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet);

    /**
     * \return the name of this routing protocol
     */
    std::string
    GetName () const;

  protected:

    /**
     * Whenever we encounter a new head packet we call this method. Otherwise, for data packets,
     * we use the same route that we used for the previous head packet.
     *
     * \return the Route or NULL if no route was found
     * \param source        source NoC net device
     * \param destination   destination NoC node
     * \param packet        the packet to be resolved (needed the whole packet, because
     *                      routing information is added as tags or headers)
     */
    virtual Ptr<Route>
    RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet) = 0;

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

    /**
     * the name of the routing protocol
     */
    std::string m_name;

    std::map<uint32_t, Ptr<NocNetDevice> > m_packetSourceNetDevices;

    std::map<uint32_t, Ptr<NocNetDevice> > m_packetDestinationNetDevices;
  };

} // namespace ns3

#endif /* NOCROUTINGPROTOCOL_H_ */
