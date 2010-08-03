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
#include "ns3/enum.h"

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
      .AddAttribute ("Type",
                     "the type of the packet",
                     EnumValue (NocPacket::UNKNOWN),
                     MakeEnumAccessor(&NocPacketTag::m_type),
                     MakeEnumChecker (NocPacket::HEAD, "Head", NocPacket::DATA, "Data", NocPacket::TAIL, "Tail"))
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
      .AddAttribute ("InjectionTime",
                     "the time of injection of the packet into the network",
                     TimeValue (),
                     MakeTimeAccessor (&NocPacketTag::m_injectionTime),
                     MakeTimeChecker ())
      .AddAttribute ("ReceiveTime",
                     "the time when the packet is received by the destination node",
                     TimeValue (),
                     MakeTimeAccessor (&NocPacketTag::m_receiveTime),
                     MakeTimeChecker ())
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
    return 22; // 1 + 4 + 1 + 8 + 8
  }

  void
  NocPacketTag::Serialize (TagBuffer i) const
  {
    i.WriteU8 (m_type);
    i.WriteU32 (m_headPacketUid);
    i.WriteU8 (m_packetBlocked);
    i.WriteU64 (m_injectionTime.GetNanoSeconds ());
    i.WriteU64 (m_receiveTime.GetNanoSeconds ());
  }

  void
  NocPacketTag::Deserialize (TagBuffer i)
  {
    int type = i.ReadU8 ();
    switch (type)
    {
      case 0:
        m_type = NocPacket::HEAD;
        break;
      case 1:
        m_type = NocPacket::DATA;
        break;
      case 2:
        m_type = NocPacket::TAIL;
        break;
      case 3:
      default:
        m_type = NocPacket::UNKNOWN;
        break;
    }
    m_headPacketUid = i.ReadU32 ();
    m_packetBlocked = i.ReadU8 ();
    m_injectionTime = NanoSeconds (i.ReadU64 ());
    m_receiveTime = NanoSeconds (i.ReadU64 ());
  }

  void
  NocPacketTag::Print (std::ostream &os) const
  {
    os << "type=" << m_type
        << "headPacketUid=" << m_headPacketUid
        << "packetBlocked=" << m_packetBlocked
        << "injectionTime=" << m_injectionTime
        << "receiveTime=" << m_receiveTime;
  }

  void
  NocPacketTag::SetPacketType (NocPacket::Type type)
  {
    m_type = type;
  }

  NocPacket::Type
  NocPacketTag::GetPacketType () const
  {
    return m_type;
  }

  void
  NocPacketTag::SetPacketHeadUid (uint32_t uid)
  {
    m_headPacketUid = uid;
  }

  uint32_t
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

  void
  NocPacketTag::SetInjectionTime (Time injectionTime)
  {
    m_injectionTime = injectionTime;
  }

  Time
  NocPacketTag::GetInjectionTime () const
  {
    return m_injectionTime;
  }

  void
  NocPacketTag::SetReceiveTime (Time receiveTime)
  {
    m_receiveTime = receiveTime;
  }

  Time
  NocPacketTag::GetReceiveTime () const
  {
    return m_receiveTime;
  }

}  // namespace ns3
