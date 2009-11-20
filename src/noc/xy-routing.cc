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

#include "xy-routing.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("XyRouting");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (XyRouting);

  TypeId
  XyRouting::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::XyRouting")
        .SetParent<NocRoutingProtocol> ();
    return tid;
  }

  // we could easily name the protocol "XY", but using __FILE__ should be more useful for debugging
  XyRouting::XyRouting() : NocRoutingProtocol(__FILE__)
  {

  }

  XyRouting::~XyRouting()
  {

  }

  bool
  XyRouting::RequestRoute(uint32_t sourceIface, const Mac48Address source,
      const Mac48Address destination, Ptr<Packet> packet,
      uint16_t protocolType, RouteReplyCallback routeReply)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_LOG_DEBUG("source " << source << " destination " << destination << " packet (on next line)");
    // FIXME print the packet only for DEBUG log level
    packet->Print(std::clog);
    std::clog << std::endl;

    routeReply (true, packet, source, destination, protocolType);
    return true;
  }

} // namespace ns3
