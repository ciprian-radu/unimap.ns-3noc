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
#include <stdint.h>
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/noc-registry.h"

NS_LOG_COMPONENT_DEFINE ("NocHeader");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED ( NocHeader);

  NocHeader::NocHeader ()
  {
    IntegerValue integerValue;
    NocRegistry::GetInstance ()->GetAttribute ("NoCDimensions", integerValue);
    uint32_t topologyDimension = integerValue.Get ();
    NocHeader::m_source = vector<uint8_t> ();
    NocHeader::m_distance = vector<uint8_t> ();
    for (unsigned int i = 0; i < topologyDimension; i ++)
      {
        NocHeader::m_source.insert(m_source.end(), 0);
        NocHeader::m_distance.insert(m_distance.end(), 0);
      }

    //    NocHeader::m_subdataId = 0;
    //    NocHeader::m_peGroupAddress = 0;
  }
  NocHeader::NocHeader (uint8_t xDistance, uint8_t yDistance, uint8_t sourceX, uint8_t sourceY, uint16_t dataFlitCount)
    {
      IntegerValue integerValue;
      NocRegistry::GetInstance ()->GetAttribute ("NoCDimensions", integerValue);
      uint32_t topologyDimension = integerValue.Get ();
      NocHeader::m_source = vector<uint8_t> ();
      NocHeader::m_distance = vector<uint8_t> ();
      for (unsigned int i = 0; i < topologyDimension; i ++)
        {
          NocHeader::m_source.insert(m_source.end(), 0);
          NocHeader::m_distance.insert(m_distance.end(), 0);
        }
      NocHeader::m_distance.at(0) = xDistance;
      NocHeader::m_distance.at(1) = yDistance;
      NocHeader::m_source.at(0) = sourceX;
      NocHeader::m_source.at(1) = sourceY;
  //    NocHeader::m_subdataId = 0;
  //    NocHeader::m_peGroupAddress = 0;
    }

  NocHeader::NocHeader (uint8_t xDistance, uint8_t yDistance, uint8_t zDistance, uint8_t sourceX, uint8_t sourceY, uint8_t sourceZ, uint16_t dataFlitCount)
  {
    IntegerValue integerValue;
    NocRegistry::GetInstance ()->GetAttribute ("NoCDimensions", integerValue);
    uint32_t topologyDimension = integerValue.Get ();
    NocHeader::m_source = vector<uint8_t> ();
    NocHeader::m_distance = vector<uint8_t> ();
    for (unsigned int i = 0; i < topologyDimension; i ++)
      {
        NocHeader::m_source.insert(m_source.end(), 0);
        NocHeader::m_distance.insert(m_distance.end(), 0);
      }
    NocHeader::m_distance.at (0) = xDistance;
    NocHeader::m_distance.at (1) = yDistance;
    NocHeader::m_distance.at (2) = zDistance;
    NocHeader::m_source.at (0) = sourceX;
    NocHeader::m_source.at (1) = sourceY;
    NocHeader::m_source.at (2) = sourceZ;
    //    NocHeader::m_subdataId = 0;
    //    NocHeader::m_peGroupAddress = 0;
  }
  NocHeader::NocHeader (std::vector<uint8_t> distance, std::vector<uint8_t> source, uint16_t dataFlitCount)
  {
    NocHeader::m_distance = distance;
    NocHeader::m_source = source;
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
    return GetHeaderSize (); // bytes
  }

  void
  NocHeader::Serialize (Buffer::Iterator start) const
  {
    // The 1 byte-constant (the first 2 bits are 1 and the rest are 0;
    // the 6 zeroes represent the packet type)
    //    start.WriteU8 (HEADER_ID);

    for (unsigned int i = 0; i < m_distance.size (); i++)
      {
        start.WriteU8 (m_distance.at (i));
      }
    for (unsigned int i = 0; i < m_source.size (); i++)
      {
        start.WriteU8 (m_source.at (i));
      }

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
    for (unsigned int i = 0; i < m_distance.size (); i++)
      {
        m_distance.at (i) = start.ReadU8 ();
      }

    for (unsigned int i = 0; i < m_source.size (); i++)
      {
        m_source.at (i) = start.ReadU8 ();
      }


    //        m_subdataId = start.ReadU8 ();

    //        m_peGroupAddress = start.ReadNtohU16 ();

    //      }

    return GetHeaderSize (); // the number of bytes consumed.
  }

  void
  NocHeader::Print (std::ostream &os) const
  {
    std::vector<string> dir(m_distance.size());
    for (unsigned int i = 0; i < m_distance.size (); i++)
      {
        if ((m_distance.at (i) & DIRECTION_BIT_MASK) == DIRECTION_BIT_MASK && (m_distance.at (i) != DIRECTION_BIT_MASK))
          {
            dir.at(i)= "B";
          }
        else
          {
            if ((m_distance.at (i) != 0) && (m_distance.at (i) != DIRECTION_BIT_MASK))
              {
                dir.at(i)= "F";
              }
          }
        os << "dimension " << (i+1) << "=<" << (int) (m_distance.at (i) & OFFSET_BIT_MASK) << ", " << dir.at (i) << "> ";
      }
    for (unsigned int i = 0; i < m_source.size (); i++)
      {
        os <<" dimension " << (i+1) << " source=" << (int) m_source.at (i);
      }
  }

  bool
  NocHeader::IsEmpty () const
  {
    bool empty = true;

    for (unsigned int i = 0; i < m_distance.size (); i++)
      {
        if (m_distance.at (i) != 0)
          {
            empty = false;
            break;
          }
      }

    for (unsigned int i = 0; i < m_source.size (); i++)
      {
        if (m_source.at (i) != 0)
          {
            empty = false;
            break;
          }
      }

    return empty;
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
    return (m_distance.at(0) & DIRECTION_BIT_MASK) != DIRECTION_BIT_MASK;
  }

  bool
  NocHeader::HasWestDirection ()
  {
    return !HasEastDirection ();
  }

  bool
  NocHeader::HasNorthDirection ()
  {
    return (m_distance.at(1) & DIRECTION_BIT_MASK) != DIRECTION_BIT_MASK;
  }

  bool
  NocHeader::HasSouthDirection ()
  {
    return !HasNorthDirection();
  }

  bool
  NocHeader::HasUpDirection ()
  {
    return (m_distance.at(2) & DIRECTION_BIT_MASK) != DIRECTION_BIT_MASK;
  }

  bool
  NocHeader::HasDownDirection ()
  {
    return !HasUpDirection ();
  }

  bool
  NocHeader::HasForwardDirection (int index)
  {
    return (m_distance.at (index) & DIRECTION_BIT_MASK) != DIRECTION_BIT_MASK;
  }

  bool
  NocHeader::HasBackDirection (int index)
  {
    return !HasForwardDirection (index);
  }

  void
  NocHeader::SetXOffset (uint8_t xOffset)
  {
    if (HasEastDirection ())
      {
        m_distance.at(0) = xOffset;
      }
    else
      {
        m_distance.at(0) = xOffset | DIRECTION_BIT_MASK;
      }
  }

  void
  NocHeader::SetYOffset (uint8_t yOffset)
  {
    if (HasNorthDirection ())
      {
        m_distance.at(1) = yOffset;
      }
    else
      {
        m_distance.at(1) = yOffset | DIRECTION_BIT_MASK;
      }
  }

  void
  NocHeader::SetZOffset (uint8_t zOffset)
  {
    if (HasUpDirection ())
      {
        m_distance.at(2) = zOffset;
      }
    else
      {
        m_distance.at(2) = zOffset | DIRECTION_BIT_MASK;
      }
  }

  void
  NocHeader::SetOffset (vector<uint8_t> offset)
  {
    for (unsigned int i = 0; i < m_distance.size (); i++)
      {
        if (HasForwardDirection (i))
          {
            m_distance.at (i) = offset.at (i);
          }
        else
          {
            m_distance.at (i) = offset.at (i) | DIRECTION_BIT_MASK;
          }
      }
  }

  uint8_t
  NocHeader::GetXOffset ()
  {
    return m_distance.at(0) & OFFSET_BIT_MASK;
  }

  uint8_t
  NocHeader::GetYOffset ()
  {
    return m_distance.at(1) & OFFSET_BIT_MASK;
  }

  uint8_t
  NocHeader::GetZOffset ()
  {
    return m_distance.at(2) & OFFSET_BIT_MASK;
  }

  vector<uint8_t>
  NocHeader::GetOffset ()
  {
    std::vector<uint8_t> distance;

    for (unsigned int i = 0; i < m_distance.size (); i++)
      {
        distance.insert (distance.end (), m_distance.at (i) & OFFSET_BIT_MASK);
      }
    return distance;
  }

  void
  NocHeader::SetSourceX (uint8_t sourceX)
  {
    m_source.at(0) = sourceX;
  }

  uint8_t
  const
  NocHeader::GetSourceX ()
  {
    return m_source.at(0);
  }

  void
  NocHeader::SetSourceY (uint8_t sourceY)
  {
    m_source.at(1) = sourceY;
  }

  uint8_t
  const
  NocHeader::GetSourceY ()
  {
    return m_source.at(1);
  }

  void
  NocHeader::SetSourceZ (uint8_t sourceZ)
  {
    m_source.at(2) = sourceZ;
  }

  uint8_t
  const
  NocHeader::GetSourceZ ()
  {
    return m_source.at(2);
  }

  void
  NocHeader::SetSource (vector<uint8_t> source)
  {
    m_source = source;
  }

  vector<uint8_t>
  const
  NocHeader::GetSource ()
  {
    return m_source;
  }

  uint32_t
  const
  NocHeader::GetHeaderSize ()
  {
    IntegerValue integerValue;
    NocRegistry::GetInstance ()->GetAttribute ("NoCDimensions", integerValue);
    uint32_t topologyDimension = integerValue.Get ();
    return (2 * topologyDimension);
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
