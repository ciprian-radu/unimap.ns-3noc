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
      m_dataFlitCount = 0;
      m_load = 0;
      m_packetBlocked = false;
      m_ctgIteration = 0;
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
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("DataFlitCount",
                     "the number of data flits from the packet",
                     EmptyAttributeValue (),
                     MakeUintegerAccessor (&NocPacketTag::GetDataFlitCount),
                     MakeUintegerChecker<uint16_t> ())
      .AddAttribute ("Load",
                     "the load of a router (as a percentage number), propagated with the packet that has this header",
                     EmptyAttributeValue (),
                     MakeUintegerAccessor (&NocPacketTag::GetLoad),
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
      .AddAttribute ("CtgIteration",
    		  	     "the CTG iteration for which this packet was created (useful for NoC Communication Task Graph based traffic generator)",
    		  	     EmptyAttributeValue (),
    		  	     MakeUintegerAccessor (&NocPacketTag::m_ctgIteration),
    		  	     MakeUintegerChecker<uint64_t> ())
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
    return 33; // 1 + 4 + 2 + 1 + 1 + 8 + 8 + 8
  }

  void
  NocPacketTag::Serialize (TagBuffer i) const
  {
    i.WriteU8 (m_type);
    i.WriteU32 (m_headPacketUid);
    i.WriteU16 (m_dataFlitCount);
    i.WriteU8 (m_load);
    i.WriteU8 (m_packetBlocked);
    i.WriteU64 (m_injectionTime.GetPicoSeconds ());
    i.WriteU64 (m_receiveTime.GetPicoSeconds ());
    i.WriteU64 (m_ctgIteration);
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
    m_dataFlitCount = i.ReadU16 ();
    m_load = i.ReadU8 ();
    m_packetBlocked = i.ReadU8 ();
    m_injectionTime = PicoSeconds (i.ReadU64 ());
    m_receiveTime = PicoSeconds (i.ReadU64 ());
    m_ctgIteration = i.ReadU64 ();
  }

  void
  NocPacketTag::Print (std::ostream &os) const
  {
    os << "type=" << m_type
       << "dataFlitCount=" << m_dataFlitCount
       << "load=" << m_load
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
  NocPacketTag::SetDataFlitCount (uint16_t dataFlitCount)
  {
    m_dataFlitCount = dataFlitCount;
  }

  uint16_t
  NocPacketTag::GetDataFlitCount () const
  {
    return m_dataFlitCount;
  }

  void
  NocPacketTag::SetLoad (uint8_t load)
  {
    m_load = load;
  }

  uint8_t
  NocPacketTag::GetLoad () const
  {
    return m_load;
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

  uint64_t
  NocPacketTag::GetCtgIteration () const
  {
	  return m_ctgIteration;
  }

  void
  NocPacketTag::SetCtgIteration (uint64_t ctgIteration)
  {
	  m_ctgIteration = ctgIteration;
  }

}  // namespace ns3
