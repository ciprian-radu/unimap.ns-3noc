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

#include "vct-switching.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"

NS_LOG_COMPONENT_DEFINE ("VctSwitching");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (VctSwitching);

  TypeId
  VctSwitching::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::VctSwitching")
        .SetParent<NocSwitchingProtocol> ()
        .AddConstructor<VctSwitching> ();
    return tid;
  }

  // we could easily name the protocol "Store and forward", but using __FILE__ should be more useful for debugging
  VctSwitching::VctSwitching() : NocSwitchingProtocol(__FILE__)
  {

  }

  VctSwitching::~VctSwitching()
  {

  }

  bool
  VctSwitching::ApplyFlowControl(Ptr<Packet> packet, Ptr<Queue> bufferedPackets)
  {
    bool canDoRouting = true;

    NocHeader header;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        packet->PeekHeader (header);
      }

    if (!header.IsEmpty())
      {
        // head packet
        if (tag.GetPacketBlocked ())
          {
            NS_LOG_DEBUG ("The head packet " << packet << " (UID " << packet->GetUid () << ") is blocked");
            m_blockedHeadFlits.insert (packet->GetUid ());
            m_flitCount.erase (packet->GetUid ());
            m_flitCount.insert (std::pair<uint32_t , uint32_t> (packet->GetUid () , (tag.GetDataFlitCount())));
            NS_LOG_LOGIC ("The packet with UID " << packet->GetUid () << " will be sent forward only after "
                << (tag.GetDataFlitCount()) << " data (body) packets will be received");
            canDoRouting = false;
          }
        else
          {
            NS_LOG_DEBUG ("The head packet " << packet << " (UID " << packet->GetUid () << ") is unblocked");
          }
      }
    if (header.IsEmpty())
      {
        // data (body) packet
        uint32_t headPacketUid = tag.GetPacketHeadUid ();
        std::set<uint32_t>::iterator it = m_blockedHeadFlits.find (headPacketUid);
        if (it != m_blockedHeadFlits.end ())
          {
            NS_LOG_DEBUG ("The data (body) packet " << packet << " (UID " << packet->GetUid () << ") is blocked");
            uint32_t dataFlitCount = m_flitCount[headPacketUid];
            NS_ASSERT (dataFlitCount >= 0);
            m_flitCount.erase (headPacketUid);
            if ((int) dataFlitCount - 1 > 0)
              {
                m_flitCount.insert (std::pair<uint32_t , uint32_t> (headPacketUid , (int) dataFlitCount - 1));
                NS_LOG_LOGIC ("Still need to receive " << ((int) dataFlitCount - 1)
                    << " data (body) packets until head packet "
                    << (int) headPacketUid << " can be sent forward");
                canDoRouting = false;
              }
            else
              {
                m_blockedHeadFlits.erase (headPacketUid);
                NS_LOG_LOGIC ("Packet with UID " << (int) headPacketUid << " can now be sent forward");
                canDoRouting = true;
              }
          }
        else
          {
            NS_LOG_DEBUG ("The data (body) packet " << packet << " (UID " << packet->GetUid () << ") is unblocked");
          }
      }
    return canDoRouting;
  }

} // namespace ns3
