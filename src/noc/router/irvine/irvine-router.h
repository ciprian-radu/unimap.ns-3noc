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
    GetTypeId();

    IrvineRouter();

    virtual
    ~IrvineRouter();

    virtual bool
    RequestRoute(const Ptr<NocNode> source, const Ptr<NocNode> destination,
        Ptr<Packet> packet, RouteReplyCallback routeReply);

  private:

    Ptr<NocNetDevice>
    GetNetDevice(const Ptr<NocNode> node, const int routingDirection);
  };

} // namespace ns3

#endif /* IRVINEROUTER_H_ */
