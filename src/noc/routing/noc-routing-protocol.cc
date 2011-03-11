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

#include "ns3/log.h"
#include "noc-routing-protocol.h"
#include "ns3/noc-node.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"

NS_LOG_COMPONENT_DEFINE ("NocRoutingProtocol");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocRoutingProtocol);

  NocRoutingProtocol::RoutingDirection
  NocRoutingProtocol::GetOpositeRoutingDirection (int direction)
  {
    switch (direction)
      {
        case FORWARD:
          return BACK;
        case BACK:
          return FORWARD;
        case NONE:
        default:
          return NONE;
      }
  }

  NocRoutingProtocol::NocRoutingProtocol (std::string name)
  {
    m_name = name;
    m_dataPacketsToBeRouted = 0;
    m_dataPacketsRouted = 0;
  }

  TypeId
  NocRoutingProtocol::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::NocRoutingProtocol")
        .SetParent<Object> ();
    return tid;
  }

  NocRoutingProtocol::~NocRoutingProtocol ()
  {
    ;
  }

  std::string
  NocRoutingProtocol::GetName () const
  {
   return m_name;
  }

  Ptr<Route>
  NocRoutingProtocol::RequestRoute (const Ptr<NocNetDevice> source,
      const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    //    Ptr<NocPacket> nocPacket = DynamicCast<NocPacket> (packet);
    //    if (nocPacket->IsHeadPacket())

    // NOTE: the above solution doens't work because the dynamic cast is useless
    // a Packet is received here, not a NocPacket...
    // Also, for the dynamic_cast to work, the Packet class must be made polymorphic
    // (this implies changing the source code of the Packet class, by introducing)
    // a virtual destructor

    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        // head packet
        NS_LOG_DEBUG ("A route was requested for the head packet " << *packet
            << " (packet UID " << packet->GetUid () << " ; current net device is "
            << source->GetAddress ()  << " ; current node is " << source->GetNode ()->GetId ()
            << " ; destination node is " << destination->GetId () << ")");

        // TODO the intention with the asserts is good but we need counters per <source, destination> pairs

//        NS_ASSERT_MSG (m_dataPacketsToBeRouted - m_dataPacketsRouted == 0,
//            "Received a new head packet but there are still " << (m_dataPacketsToBeRouted - m_dataPacketsRouted)
//            << " (out of " << m_dataPacketsToBeRouted << ") data packets to be routed!");
//        m_dataPacketsToBeRouted = header.GetDataFlitCount() - 1; // one packet is the header
//        m_dataPacketsRouted = 0;
//        NS_LOG_DEBUG ("After this head packet is routed, " << m_dataPacketsToBeRouted
//            << " data packets are still expected to be routed");
        Ptr<Route> route = RequestNewRoute(source, destination, packet);
        m_packetSourceNetDevices.erase (packet->GetUid ());
        m_packetSourceNetDevices
          .insert(std::pair<uint32_t, Ptr<NocNetDevice> > (packet->GetUid (), m_sourceNetDevice));
        NS_ASSERT (m_packetSourceNetDevices[packet->GetUid ()] != 0);
        m_packetDestinationNetDevices.erase (packet->GetUid ());
        m_packetDestinationNetDevices
          .insert(std::pair<uint32_t, Ptr<NocNetDevice> > (packet->GetUid (), m_destinationNetDevice));
        NS_ASSERT (m_packetDestinationNetDevices[packet->GetUid ()] != 0);
        return route;
      }
    else
      {
        // data (body) packet
        NocPacketTag tag;
        packet->PeekPacketTag(tag);
        NS_LOG_DEBUG ("A route was requested for the data packet " << *packet << " (head packet UID "
            << (int) tag.GetPacketHeadUid () << " ; current net device is "
            << source->GetAddress ()  << " ; current node is " << source->GetNode ()->GetId ()
            << " ; destination node is " << destination->GetId () << ")");
        Ptr<NocNetDevice> sourceDevice = m_packetSourceNetDevices[tag.GetPacketHeadUid ()];
        Ptr<NocNetDevice> destinationDevice = m_packetDestinationNetDevices[tag.GetPacketHeadUid ()];
        NS_ASSERT (sourceDevice != 0);
        NS_ASSERT (destinationDevice != 0);
//        m_dataPacketsRouted++;
//        NS_LOG_DEBUG (m_dataPacketsRouted << " were routed. Still expecting "
//            << (m_dataPacketsToBeRouted - m_dataPacketsRouted) << " data packets");
        return CreateObject<Route> (packet, sourceDevice, destinationDevice);
      }
  }

} // namespace ns3
