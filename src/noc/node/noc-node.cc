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
  NocNode::Send (Ptr<NocNetDevice> source, Ptr<Packet> packet, Ptr<NocNode> destination)
  {
    GetRouter ()->RequestRoute (source, destination, packet, MakeCallback(&NocNode::DoSend, this));
  }

  void
  NocNode::DoSend (Ptr<Packet> packet, Ptr<NetDevice> source, Ptr<NetDevice> destination)
  {
    NS_LOG_DEBUG ("Node " << GetId() << " requests net device " << source->GetAddress ()
        << " to send a packet to net device " << destination->GetAddress ()
        << " (node " << destination->GetNode ()->GetId () << ")");
    source->Send (packet, destination->GetAddress (), 0);
  }


} // namespace ns3

