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

#include "irvine-router.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"

NS_LOG_COMPONENT_DEFINE ("IrvineRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (IrvineRouter);

  TypeId
  IrvineRouter::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::IrvineRouter")
        .SetParent<NocRouter> ();
    return tid;
  }

  // we could easily name the router "Irvine router", but using __FILE__ should be more useful for debugging
  IrvineRouter::IrvineRouter() : NocRouter (__FILE__)
  {

  }

  IrvineRouter::~IrvineRouter()
  {

  }

  bool
  IrvineRouter::RequestRoute(const Ptr<NocNode> source, const Ptr<NocNode> destination,
      Ptr<Packet> packet, RouteReplyCallback routeReply)
  {
    NS_LOG_FUNCTION_NOARGS();
    GetRoutingProtocol()->RequestRoute (source, destination, packet, routeReply);
    return true;
  }

  Ptr<NocNetDevice>
  IrvineRouter::GetNetDevice(const Ptr<NocNode> node, const int routingDirection)
  {
    Ptr<NocNetDevice> netDevice = 0;
    for (unsigned int i = 0; i < node->GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> tmpNetDevice = node->GetDevice (i)->GetObject<NocNetDevice> ();
        if (tmpNetDevice->GetRoutingDirection () == routingDirection)
          {
            netDevice = tmpNetDevice;
            break;
          }
      }
    return netDevice;
  }

} // namespace ns3
