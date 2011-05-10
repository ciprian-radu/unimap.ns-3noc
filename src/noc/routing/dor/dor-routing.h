/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *                                                              Lucian Blaga University of Sibiu, Romania
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
 * Authors: Andreea Gancea <andreea.gancea@ulbsibiu.ro>
 *          Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *                      http://webspace.ulbsibiu.ro/ciprian.radu/
 */

#ifndef DOR_ROUTING_H_
#define DOR_ROUTING_H_

#include "ns3/noc-routing-protocol.h"
#include "ns3/noc-net-device.h"
#include "ns3/object-vector.h"
#include "ns3/noc-value.h"
#include "ns3/object-vector.h"


namespace ns3
{

  /**
   * \brief Dimension Order Routing for nD mesh/tor topologies.
   */
  class DorRouting : public NocRoutingProtocol
  {
  public:

    static TypeId
    GetTypeId ();

    /**
     * Constructor
     */
    DorRouting ();

    DorRouting (std::vector<Ptr<NocValue> > m_routingOrder);

    virtual
    ~DorRouting ();

    virtual Ptr<Route>
    RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet);

    void
    SetRoute (std::vector<Ptr<NocValue> > route);

    std::vector<Ptr<NocValue> >
    GetRoute () const;

  private:

    /**
     * in what order the dimensions are routed (a permutation of the numbers from 0 to n, where n is the dimensions number)
     */
    std::vector<Ptr<NocValue> > m_routingOrder;

  };
} // namespace ns3

#endif /* Dor_Routing_H_ */

