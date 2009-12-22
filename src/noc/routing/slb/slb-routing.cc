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

#include "slb-routing.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"

NS_LOG_COMPONENT_DEFINE ("SlbRouting");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (SlbRouting);

  TypeId
  SlbRouting::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::SlbRouting")
        .SetParent<NocRoutingProtocol> ();
    return tid;
  }

  // we could easily name the protocol "Static Load Bound", but using __FILE__ should be more useful for debugging
  SlbRouting::SlbRouting () : NocRoutingProtocol (__FILE__)
  {
    progressiveWeight = 2;
    remainingWeight = -1;
    loadWeight = -4;
  }

  SlbRouting::~SlbRouting ()
  {
    ;
  }

  bool
  SlbRouting::RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination,
      Ptr<Packet> packet, RouteReplyCallback routeReply)
  {
    NS_LOG_FUNCTION_NOARGS ();

    return true;
  }

} // namespace ns3
