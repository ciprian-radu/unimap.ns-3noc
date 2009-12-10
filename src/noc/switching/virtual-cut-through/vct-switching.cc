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
        .SetParent<NocSwitchingProtocol> ();
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
    packet->PeekHeader (header);

    NocPacketTag tag;
    packet->PeekPacketTag (tag);

    if (tag.GetPacketBlocked ())
      {
        NS_LOG_DEBUG ("The packet " << packet << " (UID " << packet->GetUid () << ") is blocked");
        if (!header.IsEmpty())
          {
            // head packet
            m_packetCount.erase (packet->GetUid ());
            m_packetCount.insert (std::pair<uint32_t , uint32_t> (packet->GetUid () , (header.GetDataFlitCount() - 1)));
            NS_LOG_LOGIC ("The packet with UID " << packet->GetUid () << " will be sent forward only after "
                << (header.GetDataFlitCount() - 1) << " data (body) packets will be received");
            canDoRouting = false;
          }
        else
          {
            // data (body) packet
            uint32_t v = tag.GetPacketHeadUid ();
            uint32_t dataFlitCount = m_packetCount[v];
            NS_ASSERT (dataFlitCount >= 0);
            m_packetCount.erase (v);
            if ((int) dataFlitCount - 1 > 0)
              {
                m_packetCount.insert (std::pair<uint32_t , uint32_t> (v , (int) dataFlitCount - 1));
                NS_LOG_LOGIC ("Still need to receive " << ((int) dataFlitCount - 1)
                    << " data (body) packets until head packet "
                    << (int) v << " can be sent forward");
                canDoRouting = false;
              }
            else
              {
                NS_LOG_LOGIC ("Packet with UID " << (int) v << " can now be sent forward");
                canDoRouting = true;
              }
          }
      }
    else
      {
        // notblocked instead of blocked for easy debugging...
        NS_LOG_DEBUG ("The packet " << packet << " (UID " << packet->GetUid () << ") is notblocked");
      }
    return canDoRouting;
  }

} // namespace ns3
