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

#include "noc-packet-tag.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("NocPacketTag");

namespace ns3 {

  NocPacketTag::NocPacketTag ()
    {
      m_packetBlocked = false;
    }

  TypeId
  NocPacketTag::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::NocPacketTag")
      .SetParent<Tag> ()
      .AddConstructor<NocPacketTag> ()
      .AddAttribute ("HeadPacketUid",
                     "Head packet UID",
                     EmptyAttributeValue (),
                     MakeUintegerAccessor (&NocPacketTag::GetPacketHeadUid),
                     MakeUintegerChecker<uint8_t> ())
      .AddAttribute ("PacketBlocked",
                     "Packet blocked",
                     BooleanValue (false),
                     MakeBooleanAccessor (&NocPacketTag::GetPacketBlocked),
                     MakeBooleanChecker ())
      ;
    return tid;
  }

  TypeId
  NocPacketTag::GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  uint32_t
  NocPacketTag::GetSerializedSize () const
  {
    return 2;
  }

  void
  NocPacketTag::Serialize (TagBuffer i) const
  {
    i.WriteU8 (m_headPacketUid);
    i.WriteU8 (m_packetBlocked);
  }

  void
  NocPacketTag::Deserialize (TagBuffer i)
  {
    m_headPacketUid = i.ReadU8 ();
    m_packetBlocked = i.ReadU8 ();
  }

  void
  NocPacketTag::Print (std::ostream &os) const
  {
    os << "v=" << (uint8_t) m_headPacketUid;
  }

  void
  NocPacketTag::SetPacketHeadUid (uint8_t uid)
  {
    m_headPacketUid = uid;
  }

  uint8_t
  NocPacketTag::GetPacketHeadUid () const
  {
    return m_headPacketUid;
  }

  void
  NocPacketTag::SetPacketBlocked (bool blocked)
  {
    m_packetBlocked = blocked;
  }

  bool
  NocPacketTag::GetPacketBlocked () const
  {
    return m_packetBlocked;
  }

}  // namespace ns3
