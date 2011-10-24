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

#include "saf-switching.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"

NS_LOG_COMPONENT_DEFINE ("SafSwitching");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (SafSwitching);

  TypeId
  SafSwitching::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::SafSwitching")
        .SetParent<NocSwitchingProtocol> ()
        .AddConstructor<SafSwitching> ();
    return tid;
  }

  // we could easily name the protocol "Store and forward", but using __FILE__ should be more useful for debugging
  SafSwitching::SafSwitching() : NocSwitchingProtocol(__FILE__)
  {

  }

  SafSwitching::~SafSwitching()
  {

  }

  bool
  SafSwitching::ApplyFlowControl(Ptr<Packet> packet, Ptr<Queue> bufferedPackets)
  {
    bool canDoRouting = true;
    NocHeader header;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        packet->PeekHeader(header);
      }
    if (!header.IsEmpty())
      {
        // head packet
        m_flitCount.erase (packet->GetUid ());
        m_flitCount.insert (std::pair<uint32_t , uint32_t> (packet->GetUid () , (tag.GetDataFlitCount())));
        NS_LOG_LOGIC ("The packet with UID " << packet->GetUid () << " will be sent forward only after "
            << (tag.GetDataFlitCount()) << " data (body) packets will be received");
        canDoRouting = false;
      }
    else
      {
        // data (body) packet
        NocPacketTag tag;
        packet->PeekPacketTag (tag);
        uint32_t v = tag.GetPacketHeadUid ();
        uint32_t dataFlitCount = m_flitCount[v];
        NS_ASSERT (dataFlitCount >= 0);
        m_flitCount.erase (v);
        if ((int) dataFlitCount - 1 > 0)
          {
            m_flitCount.insert (std::pair<uint32_t , uint32_t> (v , (int) dataFlitCount - 1));
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
    return canDoRouting;
  }

} // namespace ns3
