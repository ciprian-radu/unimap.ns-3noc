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
 * Authors: Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *         		http://webspace.ulbsibiu.ro/ciprian.radu/
 *          Andreea Gancea <andreea.gancea@ulbsibiu.ro>
 */

#include "xy-routing.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/boolean.h"

NS_LOG_COMPONENT_DEFINE ("XyRouting");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED ( XyRouting);

  TypeId
  XyRouting::GetTypeId ()
  {
	static TypeId tid = TypeId("ns3::XyRouting")
		.SetParent<NocRoutingProtocol> ()
		.AddConstructor<XyRouting> ()
		.AddAttribute ("RouteXFirst",
			"whether or not the X dimension is routed first",
			BooleanValue (true),
			MakeBooleanAccessor(&XyRouting::SetRouteXFirst, &XyRouting::GetRouteXFirst),
			MakeBooleanChecker ())
		;
    return tid;
  }

  // we could easily name the protocol "XY", but using __FILE__ should be more useful for debugging
  XyRouting::XyRouting () : NocRoutingProtocol (__FILE__)
  {
    m_routeXFirst = true;
    NS_LOG_DEBUG ("XY routing with X dimension routed first");
  }

  XyRouting::XyRouting (bool routeXFirst) : NocRoutingProtocol (__FILE__)
  {
    m_routeXFirst = routeXFirst;
    NS_LOG_DEBUG ("XY routing with " << (routeXFirst ? "X" : "Y") << " dimension routed first");
  }

  XyRouting::~XyRouting ()
  {
    ;
  }

  void
  XyRouting::SetRouteXFirst (bool routeXFirst)
  {
    NS_LOG_DEBUG ("XY routing with " << (routeXFirst ? "X" : "Y") << " dimension routed first");
    m_routeXFirst = routeXFirst;
  }

  bool
  XyRouting::GetRouteXFirst () const
  {
    return m_routeXFirst;
  }

  Ptr<Route>
  XyRouting::RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();
    std::stringstream ss;
    packet->Print (ss);
    NS_LOG_DEBUG ("source node = " << source->GetNode ()->GetId () << ", destination node = " << destination->GetId ()
        << ", packet " << ss.str ());
    ss.str ("");

    NocHeader nocHeader;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    NS_ASSERT (NocPacket::HEAD == tag.GetPacketType ());
    packet->RemoveHeader (nocHeader);

    bool isEast = nocHeader.HasEastDirection ();
    int xOffset = nocHeader.GetXOffset ();

    bool isNorth = nocHeader.HasNorthDirection ();
    int yOffset = nocHeader.GetYOffset ();

    RoutingDirection xDirection = NocRoutingProtocol::NONE;
    RoutingDirection yDirection = NocRoutingProtocol::NONE;
    NS_LOG_DEBUG ("xOffset " << xOffset << " direction " << (isEast ? "east" : "west"));
    NS_LOG_DEBUG ("yOffset " << yOffset << " direction " << (isNorth ? "north" : "south"));

	if (m_routeXFirst)
	  {
		if (xOffset != 0) // note that we prefer the X direction
		  {
			xOffset--;
			if (isEast)
			  {
				NS_ASSERT_MSG (xOffset >= 0, "A packet going to East will have the offset < 0");
				xDirection = NocRoutingProtocol::FORWARD;
			  }
			else
			  {
				NS_ASSERT_MSG (xOffset >= 0, "A packet going to West will have the offset < 0");
				xDirection = NocRoutingProtocol::BACK;
			  }
			nocHeader.SetXOffset (xOffset);
		  }
		else
		  {
			if (yOffset != 0)
			  {
				yOffset--;
				if (isNorth)
				  {
					NS_ASSERT_MSG (yOffset >= 0, "A packet going to North will have the offset < 0");
					yDirection = NocRoutingProtocol::FORWARD;
				  }
				else
				  {
					NS_ASSERT_MSG (yOffset >= 0, "A packet going to South will have the offset < 0");
					yDirection = NocRoutingProtocol::BACK;
				  }
				nocHeader.SetYOffset (yOffset);
			  }
		  }
	  }
	else
	  {
		if (yOffset != 0)
		  {
			yOffset--;
			if (isNorth)
			  {
				NS_ASSERT_MSG (yOffset >= 0, "A packet going to North will have the offset < 0");
				yDirection = NocRoutingProtocol::FORWARD;
			  }
			else
			  {
				NS_ASSERT_MSG (yOffset >= 0, "A packet going to South will have the offset < 0");
				yDirection = NocRoutingProtocol::BACK;
			  }
			nocHeader.SetYOffset (yOffset);
		  }
		else
		  {
			if (xOffset != 0) // note that we prefer the Y direction
			  {
				xOffset--;
				if (isEast)
				  {
					NS_ASSERT_MSG (xOffset >= 0, "A packet going to East will have the offset < 0");
					xDirection = NocRoutingProtocol::FORWARD;
				  }
				else
				  {
					NS_ASSERT_MSG (xOffset >= 0, "A packet going to West will have the offset < 0");
					xDirection = NocRoutingProtocol::BACK;
				  }
				nocHeader.SetXOffset (xOffset);
			  }
		  }
	  }

    NS_LOG_DEBUG ("new X offset " << (int) nocHeader.GetXOffset ());
    NS_LOG_DEBUG ("new Y offset " << (int) nocHeader.GetYOffset ());

    packet->AddHeader (nocHeader);

    packet->Print (ss);
    NS_LOG_DEBUG ("source node = " << source->GetNode ()->GetId () << ", destination node = " << destination->GetId ()
        << ", packet " << ss.str ());
    ss.str ("");

    Ptr<Route> route = 0;

    bool routeX = true;
    bool routeY = true;
    switch (xDirection) {
		case NocRoutingProtocol::FORWARD:
		  m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetOutputNetDevice (source, FORWARD, 0);
		  NS_ASSERT (m_sourceNetDevice != 0);
		  m_destinationNetDevice = destination->GetRouter ()->GetInputNetDevice (m_sourceNetDevice, BACK, 0);
		  NS_ASSERT (m_destinationNetDevice != 0);
		  route = CreateObject<Route> (packet, m_sourceNetDevice, m_destinationNetDevice);
		  break;
		case NocRoutingProtocol::BACK:
		  m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetOutputNetDevice (source, BACK, 0);
		  NS_ASSERT (m_sourceNetDevice != 0);
		  m_destinationNetDevice = destination->GetRouter ()->GetInputNetDevice (m_sourceNetDevice, FORWARD, 0);
		  NS_ASSERT (m_destinationNetDevice != 0);
		  route = CreateObject<Route> (packet, m_sourceNetDevice, m_destinationNetDevice);
		  break;
		case NocRoutingProtocol::NONE:
		  routeX = false;
		default:
		  routeX = false;
		  break;
      }

    switch (yDirection) {
		case NocRoutingProtocol::FORWARD:
		  m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetOutputNetDevice (source, FORWARD, 1);
		  NS_ASSERT (m_sourceNetDevice != 0);
		  m_destinationNetDevice = destination->GetRouter ()->GetInputNetDevice (m_sourceNetDevice, NocRoutingProtocol::BACK, 1);
		  NS_ASSERT (m_destinationNetDevice != 0);
		  route = CreateObject<Route> (packet, m_sourceNetDevice, m_destinationNetDevice);
		  break;
		case NocRoutingProtocol::BACK:
		  m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetOutputNetDevice (source,
			  NocRoutingProtocol::BACK, 1);
		  NS_ASSERT (m_sourceNetDevice != 0);
		  m_destinationNetDevice = destination->GetRouter ()->GetInputNetDevice (m_sourceNetDevice, NocRoutingProtocol::FORWARD, 1);
		  NS_ASSERT (m_destinationNetDevice != 0);
		  route = CreateObject<Route> (packet, m_sourceNetDevice, m_destinationNetDevice);
		  break;
		case NocRoutingProtocol::NONE:
		  routeY = false;
		default:
		  routeY = false;
		  break;
      }

    if (!routeX && !routeY)
      {
        NS_LOG_WARN ("No routing needs to be performed!");
      }

    return route;
  }

} // namespace ns3
