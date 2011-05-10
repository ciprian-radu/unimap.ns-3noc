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

#ifndef NOCHEADER_H_
#define NOCHEADER_H_

#include "ns3/header.h"
#include "ns3/buffer.h"

namespace ns3
{

  /**
   * \brief Header for messages routed in 2D mesh and torus NoCs (based on the Irvine NoC architecture)
   *
   * This class has fields corresponding to those in a Network on Chip (NoC) header
   * (destination address, source address, etc).
   */
  class NocHeader : public Header
  {
  public:

    static const uint8_t DIRECTION_BIT_MASK = 0x80;

    static const uint8_t OFFSET_BIT_MASK = 0x7F;

    NocHeader ();

    /**
     * Constructor - creates a header
     *
     * \param xDistance the X (horizontal) offset to the destination
     * \param yDistance the Y (vertical) offset to the destination
     * \param sourceX the X coordinate of the source node
     * \param sourceY the Y coordinate of the source node
     * \param dataFlitCount the number of data flits (packets) that the message having this header will have
     */
    NocHeader (uint8_t xDistance, uint8_t yDistance, uint8_t sourceX, uint8_t sourceY, uint16_t dataFlitCount);

    /**
     * Constructor - creates a header
     *
     * \param xDistance the X (horizontal) offset to the destination
     * \param yDistance the Y (vertical) offset to the destination
     * \param yDistance the Z offset to the destination
     * \param sourceX the X coordinate of the source node
     * \param sourceY the Y coordinate of the source node
     * \param sourceY the Z coordinate of the source node
     * \param dataFlitCount the number of data flits (packets) that the message having this header will have
     */
    NocHeader (uint8_t xDistance, uint8_t yDistance, uint8_t zDistance,
        uint8_t sourceX, uint8_t sourceY, uint8_t sourceZ,  uint16_t dataFlitCount);

    /**
     * Constructor - creates a header
     *
     * \param distance the offset to the destination
     * \param source the coordinate of the source node
     * \param dataFlitCount the number of data flits (packets) that the message having this header will have
     */

    NocHeader (std::vector <uint8_t> distance, std::vector <uint8_t> source, uint16_t dataFlitCount);

    virtual
    ~NocHeader ();

    static TypeId
    GetTypeId ();

    virtual TypeId
    GetInstanceTypeId () const;

    virtual uint32_t
    GetSerializedSize () const;

    virtual void
    Serialize (Buffer::Iterator start) const;

    virtual uint32_t
    Deserialize (Buffer::Iterator start);

    virtual void
    Print (std::ostream &os) const;

    /**
     * \return whether or not this is actually a header or only a dummy (uninitialized) header
     */
    bool
    IsEmpty () const;

    // allow protocol-specific access to the header data.

//    void
//    SetXDistance (uint8_t xDistance);
//
//    uint8_t
//    const GetXDistance ();
//
//    void
//    SetYDistance (uint8_t yDistance);
//
//    uint8_t
//    const GetYDistance ();

    bool
    HasEastDirection ();

    bool
    HasWestDirection ();

    bool
    HasNorthDirection ();

    bool
    HasSouthDirection ();

    bool
    HasUpDirection ();

    bool
    HasDownDirection ();

    bool
    HasForwardDirection (int index);

    bool
    HasBackDirection (int index);

    void
    SetXOffset (uint8_t xOffset);

    uint8_t
    GetXOffset ();

    void
    SetYOffset (uint8_t yOffset);

    uint8_t
    GetYOffset ();

    void
    SetZOffset (uint8_t zOffset);

    uint8_t
    GetZOffset ();

    void
    SetOffset (std::vector <uint8_t> offset);

    std::vector<uint8_t>
    GetOffset ();

    void
    SetSourceX (uint8_t sourceX);

    uint8_t
    const GetSourceX ();

    void
    SetSourceY (uint8_t sourceY);

    uint8_t
    const GetSourceY ();

    void
    SetSourceZ (uint8_t sourceZ);

    uint8_t
    const GetSourceZ ();

    void
    SetSource (std::vector<uint8_t> source);

    std::vector<uint8_t>
    const GetSource ();

    /**
     * the size of this type of header, in bytes
     */
    static uint32_t
    const GetHeaderSize ();

//    void
//    SetSubdataId (uint8_t subdataId);
//
//    uint8_t
//    const GetSubdataId ();

//    void
//    SetPeGroupAddress (uint16_t peGroupAddress);
//
//    uint16_t
//    const GetPeGroupAddress ();

  public:

    /**
     * the size of this type of header, in bytes
     */
    // Make sure to update this field whenever you modify the structure of the header
//    static const uint32_t HEADER_SIZE = 6;

  private:

//    /**
//     * the identifier of this kind of header
//     */
//    static const uint8_t HEADER_ID = 0x0C;

    // m_Distance determine the destination address

    std::vector <uint8_t> m_distance;

    // m_xDistance, m_yDistance and m_zDistance determine the destination address

//    uint8_t m_xDistance;

//    uint8_t m_yDistance;

    // m_sourceX, m_sourceY and m_sourceZ for the source address

    std::vector <uint8_t> m_source;

    // m_sourceX, m_sourceY and m_sourceZ for the source address

//    uint8_t m_sourceX;

//    uint8_t m_sourceY;

//    /**
//     * allows package indexing; this is useful when data has to be split
//     * into multiple packets (a package order is required is such a case)
//     */
//    uint8_t m_subdataId;

//    /**
//     * allows to specify a group of processing elements to which the packet is sent
//     * (this allows multicasting)
//     */
//    uint16_t m_peGroupAddress;

  };

} // namespace ns3

#endif /* NOCHEADER_H_ */
