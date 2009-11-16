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

#include "noc-header.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("NoC-Header");

namespace ns3
{

  NocHeader::NocHeader()
  {
    NocHeader::m_xDistance = 0;
    NocHeader::m_yDistance = 0;
    NocHeader::m_sourceX = 0;
    NocHeader::m_sourceY = 0;
    NocHeader::m_subdataId = 0;
    NocHeader::m_peGroupAddress = 0;
    NocHeader::m_dataFlitCount = 0;
  }

  NocHeader::NocHeader(uint8_t xDistance, uint8_t yDistance, uint8_t sourceX,
      uint8_t sourceY, uint16_t dataFlitCount)
  {
    NS_ASSERT_MSG(xDistance <= 16, "xDistance must be only on 4 bits");
    NocHeader::m_xDistance = xDistance;
    NS_ASSERT_MSG(yDistance <= 16, "yDistance must be only on 4 bits");
    NocHeader::m_yDistance = yDistance;
    NS_ASSERT_MSG(sourceX <= 16, "sourceX must be only on 4 bits");
    NocHeader::m_sourceX = sourceX;
    NS_ASSERT_MSG(sourceY <= 16, "sourceY must be only on 4 bits");
    NocHeader::m_sourceY = sourceY;
    NocHeader::m_subdataId = 0;
    NocHeader::m_peGroupAddress = 0;
    NocHeader::m_dataFlitCount = dataFlitCount;
  }

  NocHeader::~NocHeader()
  {
    ;
  }

  TypeId
  NocHeader::GetTypeId (void)
  {
    static TypeId tid = TypeId ("NocHeader")
      .SetParent<Header> ()
      .AddConstructor<NocHeader> ()
    ;
    return tid;
  }

  TypeId
  NocHeader::GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  uint32_t
  NocHeader::GetSerializedSize (void) const
  {
    return 8; // bytes
  }

  void
  NocHeader::Serialize (Buffer::Iterator start) const
  {
    // The 1 byte-constant (the first 2 bits are 1 and the rest are 0;
    // the 6 zeroes represent the packet type)
    start.WriteU8 (0xC0);

    start.WriteU8 ((m_xDistance << 4) + m_yDistance);

    start.WriteU8 ((m_sourceX << 4) + m_sourceY);

    start.WriteU8 (m_subdataId);

    start.WriteHtonU16 (m_peGroupAddress);

    start.WriteHtonU16 (m_dataFlitCount);
  }

  uint32_t
  NocHeader::Deserialize (Buffer::Iterator start)
  {
    uint8_t tmp;
    tmp = start.ReadU8 ();
    NS_ASSERT (tmp == 0xC0);

    uint8_t destinationAddress = start.ReadU8 ();
    m_xDistance = destinationAddress >> 4;
    m_yDistance = destinationAddress & 0x0F;

    uint8_t sourceAddress = start.ReadU8 ();
    m_sourceX = sourceAddress >> 4;
    m_sourceY = sourceAddress & 0x0F;

    m_subdataId = start.ReadU8 ();

    m_peGroupAddress = start.ReadNtohU16 ();

    m_dataFlitCount = start.ReadNtohU16 ();

    return 8; // the number of bytes consumed.
  }

  void
  NocHeader::Print (std::ostream &os) const
  {
    os << "xDistance=" << (int) m_xDistance << " yDistance=" << (int) m_yDistance
        << " sourceX=" << (int) m_sourceX << " sourceY=" << (int) m_sourceY << " subdataId="
        << (int) m_subdataId << " peGroupAddress=" << (long) m_peGroupAddress << " dataFlitCount="
        << (long) m_dataFlitCount;
  }

  void
  NocHeader::SetXDistance(uint8_t xDistance)
  {
    NS_ASSERT_MSG(xDistance <= 16, "xDistance must be only on 4 bits");
    m_xDistance = xDistance;
  }

  uint8_t
  const NocHeader::GetXDistance()
  {
    return m_xDistance;
  }

  void
  NocHeader::SetYDistance(uint8_t yDistance)
  {
    NS_ASSERT_MSG(yDistance <= 16, "yDistance must be only on 4 bits");
    m_yDistance = yDistance;
  }

  uint8_t
  const NocHeader::GetYDistance()
  {
    return m_yDistance;
  }

  void
  NocHeader::SetSourceX(uint8_t sourceX)
  {
    NS_ASSERT_MSG(sourceX <= 16, "sourceX must be only on 4 bits");
    m_sourceX = sourceX;
  }

  uint8_t
  const NocHeader::GetSourceX()
  {
    return m_sourceX;
  }

  void
  NocHeader::SetSourceY(uint8_t sourceY)
  {
    NS_ASSERT_MSG(sourceY <= 16, "sourceY must be only on 4 bits");
    m_sourceY = sourceY;
  }

  uint8_t
  const NocHeader::GetSourceY()
  {
    return m_sourceY;
  }

  void
  NocHeader::SetSubdataId(uint8_t subdataId)
  {
    m_subdataId = subdataId;
  }

  uint8_t
  const NocHeader::GetSubdataId()
  {
    return m_subdataId;
  }

  void
  NocHeader::SetPeGroupAddress(uint16_t peGroupAddress)
  {
    m_peGroupAddress = peGroupAddress;
  }

  uint16_t
  const NocHeader::GetPeGroupAddress()
  {
    return m_peGroupAddress;
  }

  void
  NocHeader::SetDataFlitCount(uint16_t dataFlitCount)
  {
    m_dataFlitCount = dataFlitCount;
  }

  uint16_t
  const NocHeader::GetDataFlitCount()
  {
    return m_dataFlitCount;
  }

} // namespace ns3
