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
 */

#ifndef XYROUTING_H_
#define XYROUTING_H_

#include "ns3/noc-routing-protocol.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  /**
   * \brief Dimension Order Routing for 2D mesh topologies. By default, the X (horizontal) dimension
   * is preferred but, this can be changed to the Y dimension.
   */
  class XyRouting : public NocRoutingProtocol
  {
  public:

    static TypeId
    GetTypeId();

    /**
     * Constructor
     *
     * the X dimension is routed first
     */
    XyRouting();

    /**
     * Constructor
     *
     * \param routeXFirst specifies if the X dimension is routed first (Y dimension otherwise)
     */
    XyRouting(bool routeXFirst);

    virtual
    ~XyRouting();

    virtual Ptr<Route>
    RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet);

    void
    SetRouteXFirst (bool routeXFirst);

    bool
    GetRouteXFirst () const;

  private:

    bool m_routeXFirst;
  };

} // namespace ns3

#endif /* XYROUTING_H_ */
