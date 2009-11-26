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
#include "noc-header.h"

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
  XyRouting::RequestRoute(const Ptr<NocNode> source, const Ptr<NocNode> destination,
      Ptr<Packet> packet, RouteReplyCallback routeReply)
  {
    NS_LOG_FUNCTION_NOARGS();
    std::stringstream ss;
    packet->Print(ss);
    NS_LOG_DEBUG("source node = " << source->GetId () << ", destination node = " << destination->GetId ()
        << ", packet " << ss.str ());
    ss.str("");

    NocHeader nocHeader;
    packet->RemoveHeader (nocHeader);

    uint8_t xDistance = nocHeader.GetXDistance ();
    bool isEast = (xDistance & 0x08) != 0x08;
    int xOffset = xDistance & 0x07;

    uint8_t yDistance = nocHeader.GetYDistance ();
    bool isSouth = (xDistance & 0x08) != 0x08;
    int yOffset = yDistance & 0x07;

    Direction xDirection = NONE;
    Direction yDirection = NONE;
    NS_LOG_DEBUG("xOffset " << xOffset);
    NS_LOG_DEBUG("yOffset " << yOffset);
    if (xOffset != 0) // note that we prefer the X direction
      {
        xOffset--;
        if (isEast)
          {
            NS_ASSERT_MSG(xOffset >= 0, "A packet going to East will have the offset < 0");
            xDirection = EAST;
          }
        else
          {
            NS_ASSERT_MSG(xOffset >= 0, "A packet going to West will have the offset < 0");
            xDirection = WEST;
          }
        nocHeader.SetXDistance(isEast ? xOffset : xOffset | 0x08);
      }
    else
      {
        if (yOffset != 0)
          {
            yOffset--;
            if (isSouth)
              {
                NS_ASSERT_MSG(yOffset >= 0, "A packet going to South will have the offset < 0");
                yDirection = SOUTH;
              }
            else
              {
                NS_ASSERT_MSG(yOffset >= 0, "A packet going to North will have the offset < 0");
                yDirection = NORTH;
              }
            nocHeader.SetYDistance(isSouth ? yOffset : yOffset | 0x08);
          }
      }
    NS_LOG_DEBUG("new xDistance " << (int) nocHeader.GetXDistance());
    NS_LOG_DEBUG("new yDistance " << (int) nocHeader.GetYDistance());

    packet->AddHeader (nocHeader);

    packet->Print(ss);
    NS_LOG_DEBUG("source node = " << source->GetId () << ", destination node = " << destination->GetId ()
        << ", packet " << ss.str ());
    ss.str("");

    Ptr<NocNetDevice> sourceNetDevice;
    Ptr<NocNetDevice> destinationNetDevice;
    bool routeX = true;
    bool routeY = false;
    switch (xDirection) {
      case EAST:
        sourceNetDevice = GetNetDevice(source, EAST);
        NS_ASSERT(sourceNetDevice != 0);
        destinationNetDevice = GetNetDevice(destination, WEST);
        NS_ASSERT(destinationNetDevice != 0);
        routeReply (packet, sourceNetDevice, destinationNetDevice);
        break;
      case WEST:
        sourceNetDevice = GetNetDevice(source, WEST);
        NS_ASSERT(sourceNetDevice != 0);
        destinationNetDevice = GetNetDevice(destination, EAST);
        NS_ASSERT(destinationNetDevice != 0);
        routeReply (packet, sourceNetDevice, destinationNetDevice);
        break;
      case NORTH:
        NS_LOG_ERROR("A NORTH direction is not allowed as a horizontal direction");
        routeX = false;
        break;
      case SOUTH:
        NS_LOG_ERROR("A SOUTH direction is not allowed as a horizontal direction");
        routeX = false;
        break;
      case NONE:
        routeX = false;
      default:
        routeY = false;
        break;
    }

    switch (yDirection) {
      case NORTH:
        sourceNetDevice = GetNetDevice(source, NORTH);
        NS_ASSERT(sourceNetDevice != 0);
        destinationNetDevice = GetNetDevice(destination, SOUTH);
        NS_ASSERT(destinationNetDevice != 0);
        routeReply (packet, sourceNetDevice, destinationNetDevice);
        break;
      case SOUTH:
        sourceNetDevice = GetNetDevice(source, SOUTH);
        NS_ASSERT(sourceNetDevice != 0);
        destinationNetDevice = GetNetDevice(destination, NORTH);
        NS_ASSERT(destinationNetDevice != 0);
        routeReply (packet, sourceNetDevice, destinationNetDevice);
        break;
      case EAST:
        NS_LOG_ERROR("A EAST direction is not allowed as a vertical direction");
        routeY = false;
        break;
      case WEST:
        NS_LOG_ERROR("A WEST direction is not allowed as a vertical direction");
        routeY = false;
        break;
      case NONE:
        routeY = false;
      default:
        routeY = false;
        break;
    }

    if (!routeX && !routeY)
      {
        NS_LOG_WARN ("No routing needs to be performed!");
      }

    return true;
  }

  Ptr<NocNetDevice>
  XyRouting::GetNetDevice(const Ptr<NocNode> node, const int routingDirection)
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
