/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *               						Lucian Blaga University of Sibiu, Romania
 *               - Systems and Networking, University of Augsburg, Germany
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
 * Authors: Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *         		http://webspace.ulbsibiu.ro/ciprian.radu/
 *          Andreea Gancea <andreea.gancea@ulbsibiu.ro>
 */

#include "noc-header.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("NocHeader");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED ( NocHeader);

  NocHeader::NocHeader ()
  {
    NocHeader::m_xDistance = 0;
    NocHeader::m_yDistance = 0;
    NocHeader::m_sourceX = 0;
    NocHeader::m_sourceY = 0;
//    NocHeader::m_subdataId = 0;
//    NocHeader::m_peGroupAddress = 0;
  }

  NocHeader::NocHeader (uint8_t xDistance, uint8_t yDistance, uint8_t sourceX, uint8_t sourceY, uint16_t dataFlitCount)
  {
    NocHeader::m_xDistance = xDistance;
    NocHeader::m_yDistance = yDistance;
    NocHeader::m_sourceX = sourceX;
    NocHeader::m_sourceY = sourceY;
//    NocHeader::m_subdataId = 0;
//    NocHeader::m_peGroupAddress = 0;
  }

  NocHeader::~NocHeader ()
  {
    ;
  }

  TypeId
  NocHeader::GetTypeId ()
  {
	static TypeId tid = TypeId ("NocHeader")
	  .SetParent<Header> ()
	  .AddConstructor<NocHeader> ()
	;
    return tid;
  }

  TypeId
  NocHeader::GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  uint32_t
  NocHeader::GetSerializedSize () const
  {
    return HEADER_SIZE; // bytes
  }

  void
  NocHeader::Serialize (Buffer::Iterator start) const
  {
    // The 1 byte-constant (the first 2 bits are 1 and the rest are 0;
    // the 6 zeroes represent the packet type)
//    start.WriteU8 (HEADER_ID);

    start.WriteU8 (m_xDistance);

    start.WriteU8 (m_yDistance);

    start.WriteU8 (m_sourceX);

    start.WriteU8 (m_sourceY);

//    start.WriteU8 (m_subdataId);

//    start.WriteHtonU16 (m_peGroupAddress);

  }

  uint32_t
  NocHeader::Deserialize (Buffer::Iterator start)
  {
//    uint8_t tmp;
//    tmp = start.ReadU8 ();
//    // if tmp == 0 then we have a data packet
//    if (tmp != 0)
//      {
//        NS_ASSERT (tmp == HEADER_ID);

        m_xDistance = start.ReadU8 ();
        m_yDistance = start.ReadU8 ();

        m_sourceX = start.ReadU8 ();
        m_sourceY = start.ReadU8 ();

//        m_subdataId = start.ReadU8 ();

//        m_peGroupAddress = start.ReadNtohU16 ();

//      }

    return HEADER_SIZE; // the number of bytes consumed.
  }

  void
  NocHeader::Print (std::ostream &os) const
  {
    std::string xDir;
    std::string yDir;

    if ((m_xDistance & DIRECTION_BIT_MASK) == DIRECTION_BIT_MASK)
      {
        xDir = "W";
      }
    else
      {
        if (m_xDistance != 0)
          {
            xDir = "E";
          }
      }
    if ((m_yDistance & DIRECTION_BIT_MASK) == DIRECTION_BIT_MASK)
      {
        yDir = "N";
      }
    else
      {
        if (m_yDistance != 0)
          {
            yDir = "S";
          }
      }

    os << "x=<" << (int) (m_xDistance & OFFSET_BIT_MASK) << ", " << xDir << "> "
       << "y=<" << (int) (m_yDistance & OFFSET_BIT_MASK) << ", " << yDir << "> "
       << "sourceX=" << (int) m_sourceX
       << " sourceY=" << (int) m_sourceY;
//       << " subdataId=" << (int) m_subdataId
//       << " peGroupAddress=" << (long) m_peGroupAddress
  }

  bool
  NocHeader::IsEmpty () const
  {
    return (m_xDistance == 0)
        && (m_yDistance == 0)
        && (m_sourceX == 0)
        && (m_sourceY == 0);
//        && (m_subdataId == 0)
//        && (m_peGroupAddress == 0)
  }

  //  void
  //  NocHeader::SetXDistance(uint32_t xDistance)
  //  {
  //    m_xDistance = xDistance;
  //  }
  //
  //  uint32_t
  //  const NocHeader::GetXDistance()
  //  {
  //    return m_xDistance;
  //  }
  //
  //  void
  //  NocHeader::SetYDistance(uint32_t yDistance)
  //  {
  //    m_yDistance = yDistance;
  //  }
  //
  //  uint32_t
  //  const NocHeader::GetYDistance()
  //  {
  //    return m_yDistance;
  //  }

  bool
  NocHeader::HasEastDirection ()
  {
    return (m_xDistance & DIRECTION_BIT_MASK) != DIRECTION_BIT_MASK;
  }

  bool
  NocHeader::HasWestDirection ()
  {
    return !HasEastDirection ();
  }

  bool
  NocHeader::HasNorthDirection ()
  {
    return !HasSouthDirection ();
  }

  bool
  NocHeader::HasSouthDirection ()
  {
    return (m_yDistance & DIRECTION_BIT_MASK) != DIRECTION_BIT_MASK;
  }

  void
  NocHeader::SetXOffset (uint8_t xOffset)
  {
    if (HasEastDirection ())
      {
        m_xDistance = xOffset;
      }
    else
      {
        m_xDistance = xOffset | DIRECTION_BIT_MASK;
      }
  }

  void
  NocHeader::SetYOffset (uint8_t yOffset)
  {
    if (HasSouthDirection ())
      {
        m_yDistance = yOffset;
      }
    else
      {
        m_yDistance = yOffset | DIRECTION_BIT_MASK;
      }
  }

  uint8_t
  NocHeader::GetXOffset ()
  {
    return m_xDistance & OFFSET_BIT_MASK;
  }

  uint8_t
  NocHeader::GetYOffset ()
  {
    return m_yDistance & OFFSET_BIT_MASK;
  }

  void
  NocHeader::SetSourceX (uint8_t sourceX)
  {
    m_sourceX = sourceX;
  }

  uint8_t
  const
  NocHeader::GetSourceX ()
  {
    return m_sourceX;
  }

  void
  NocHeader::SetSourceY (uint8_t sourceY)
  {
    m_sourceY = sourceY;
  }

  uint8_t
  const
  NocHeader::GetSourceY ()
  {
    return m_sourceY;
  }

//  void
//  NocHeader::SetSubdataId (uint8_t subdataId)
//  {
//    m_subdataId = subdataId;
//  }
//
//  uint8_t
//  const
//  NocHeader::GetSubdataId ()
//  {
//    return m_subdataId;
//  }

//  void
//  NocHeader::SetPeGroupAddress (uint16_t peGroupAddress)
//  {
//    m_peGroupAddress = peGroupAddress;
//  }
//
//  uint16_t
//  const
//  NocHeader::GetPeGroupAddress ()
//  {
//    return m_peGroupAddress;
//  }

} // namespace ns3
