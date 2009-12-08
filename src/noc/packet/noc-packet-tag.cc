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

  TypeId
  NocPacketTag::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::NocPacketTag")
      .SetParent<Tag> ()
      .AddConstructor<NocPacketTag> ()
      .AddAttribute ("SimpleValue",
                     "A simple value",
                     EmptyAttributeValue (),
                     MakeUintegerAccessor (&NocPacketTag::GetSimpleValue),
                     MakeUintegerChecker<uint8_t> ())
      ;
    return tid;
  }

  TypeId
  NocPacketTag::GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  uint32_t
  NocPacketTag::GetSerializedSize (void) const
  {
    return 1;
  }

  void
  NocPacketTag::Serialize (TagBuffer i) const
  {
    i.WriteU8 (m_simpleValue);
  }

  void
  NocPacketTag::Deserialize (TagBuffer i)
  {
    m_simpleValue = i.ReadU8 ();
  }

  void
  NocPacketTag::Print (std::ostream &os) const
  {
    os << "v=" << (uint32_t)m_simpleValue;
  }

  void
  NocPacketTag::SetSimpleValue (uint8_t value)
  {
    m_simpleValue = value;
  }

  uint8_t
  NocPacketTag::GetSimpleValue (void) const
  {
    return m_simpleValue;
  }

}  // namespace ns3
