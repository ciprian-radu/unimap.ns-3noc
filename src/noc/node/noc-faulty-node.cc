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

#include "noc-faulty-node.h"
#include "ns3/log.h"
#include "ns3/boolean.h"

NS_LOG_COMPONENT_DEFINE ("NocFaultyNode");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocFaultyNode);

  TypeId
  NocFaultyNode::GetTypeId ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    static TypeId tid = TypeId ("ns3::NocFaultyNode")
        .SetParent<NocNode> ()
        .AddConstructor<NocFaultyNode> ()
        .AddAttribute (
            "Faulty",
            "Indicates if a node is faulty or not",
            BooleanValue (false),
            MakeBooleanAccessor (&NocFaultyNode::m_faulty),
            MakeBooleanChecker ())
        ;
    return tid;
  }

  NocFaultyNode::NocFaultyNode ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_faulty = false;
  }

  NocFaultyNode::~NocFaultyNode ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NocFaultyNode::SetRouter (Ptr<NocRouter> router)
  {
    if (!m_faulty)
      {
        NocNode::SetRouter (router);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set the router of node " << (int) GetId () << " because the node is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  Ptr<NocRouter>
  NocFaultyNode::GetRouter ()
  {
    Ptr<NocRouter> router = 0;
    if (!m_faulty)
      {
        router = NocNode::GetRouter ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot retrieve the router of node " << (int) GetId () << " because the node is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
    return router;
  }

//  void
//  NocFaultyNode::SetFaulty (bool faulty)
//  {
//    NS_LOG_FUNCTION_NOARGS ();
//    m_faulty = faulty;
//    NS_LOG_LOGIC ("The node " << (int) GetId () << " has the faulty state set to " << faulty);
//  }
//
//  bool
//  NocFaultyNode::IsFaulty ()
//  {
//    NS_LOG_FUNCTION_NOARGS ();
//    NS_LOG_LOGIC ("The node " << (int) GetId () << " has the faulty state " << m_faulty);
//    return m_faulty;
//  }

  void
  NocFaultyNode::InjectPacket (Ptr<NocPacket> packet, Ptr<NocNode> destination)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNode::InjectPacket (packet, destination);
      }
    else
      {
        NS_LOG_ERROR ("Cannot inject packet at node " << (int) GetId () << " because it is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  void
  NocFaultyNode::Send (Ptr<NocNetDevice> source, Ptr<Packet> packet, Ptr<NocNode> destination)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocNode::Send (source, packet, destination);
      }
    else
      {
        NS_LOG_ERROR ("Node " << (int) GetId () << " cannot send a packet because it is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

} // namespace ns3

