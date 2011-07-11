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

#include "noc-node.h"
#include "ns3/log.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/simulator.h"
#include "ns3/noc-registry.h"
#include "ns3/integer.h"

NS_LOG_COMPONENT_DEFINE ("NocNode");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocNode);

  TypeId
  NocNode::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::NocNode")
        .SetParent<Node> ()
        .AddConstructor<NocNode> ();
    return tid;
  }

  NocNode::NocNode() :
    m_router(0)
  {
  }

  NocNode::~NocNode()
  {
    m_router = 0;
  }

  void
  NocNode::SetRouter (Ptr<NocRouter> router)
  {
    std::ostringstream oss;
    oss << "Setting a '" << router->GetName () << "' router for the NoC node " << GetId ();
    NS_LOG_DEBUG (oss.str());
    NS_ASSERT_MSG (PeekPointer (router->GetNocNode ()) == this,
        "This router must be installed on this NoC net device to be useful.");
    m_router = router;
  }

  Ptr<NocRouter>
  NocNode::GetRouter ()
  {
    if (m_router == 0)
      {
        std::ostringstream oss;
        oss << "No router is defined for the NoC node " << GetId ();
        NS_LOG_WARN(oss.str());
      }
    return m_router;
  }

  void
  NocNode::InjectPacket (Ptr<NocPacket> packet, Ptr<NocNode> destination)
  {
    NS_LOG_LOGIC ("Node " << GetId () << " is injecting the packet " << *packet
        << " (packet UID " << packet->GetUid () << " ; destination is "
        << destination->GetId () << ")");

    NocPacketTag packetTag;
    packet->RemovePacketTag (packetTag);
    packetTag.SetInjectionTime (Simulator::Now ());
    packet->AddPacketTag (packetTag);
    Ptr<NocNetDevice> netDevice = GetRouter ()->GetInjectionNetDevice (packet, destination);
    Send (netDevice, packet, destination);
//    Simulator::ScheduleNow (&NocNode::Send, this, netDevice, packet, destination);
  }

  void
  NocNode::Send (Ptr<NocNetDevice> source, Ptr<Packet> packet, Ptr<NocNode> destination)
  {
    NS_LOG_DEBUG ("source " << source->GetAddress () << ", packet UID " << packet->GetUid () << ", destination " << destination->GetId ());

    NocHeader header;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        packet->PeekHeader (header);
      }
    Ptr<Route> route = GetRouter ()->ManageFlit (source, destination, packet);
    NS_LOG_DEBUG ("The route for packet with UID " << packet->GetUid ()
        << " is from " << route->GetSourceDevice ()->GetAddress ()
        << " to " << route->GetDestinationDevice ()->GetAddress ());
    // note the packet returned by the route has its header updated!
    // => packet->PeekHeader (...) <> route->GetPacket ()->PeekHeader (...)
    Ptr<Packet> routedPacket = route->GetRoutedPacket ();
    // add the original header as well
    if (NocPacket::HEAD == tag.GetPacketType () && !header.IsEmpty ())
      {
        routedPacket->AddHeader (header);
      }
    DoSend (routedPacket, source, route->GetSourceDevice (), route->GetDestinationDevice ());
  }

  void
  NocNode::DoSend (Ptr<Packet> packet, Ptr<NocNetDevice> source, Ptr<NocNetDevice> viaNetDevice, Ptr<NetDevice> destination)
  {
    NS_LOG_DEBUG ("Node " << GetId() << " requests net device " << source->GetAddress ()
        << " to send the packet with UID " << packet->GetUid () << " to net device "
        << destination->GetAddress () << " (node " << destination->GetNode ()->GetId () << ")");
    source->SetViaNetDevice (viaNetDevice);
    source->Send (packet, destination->GetAddress (), 0);
  }


} // namespace ns3

