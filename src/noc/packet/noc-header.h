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
   * \brief Header for messages routed in 2D mesh NoCs (based on the Irvine router architecture)
   *
   * This class has fields corresponding to those in a Network on Chip (NoC) header
   * (destination address, source address, etc).
   */
  class NocHeader : public Header
  {
  public:
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
    NocHeader (uint8_t xDistance, uint8_t yDistance, uint8_t sourceX,
        uint8_t sourceY, uint16_t dataFlitCount);

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
     * \return the size of the header, in bytes
     */
    virtual uint8_t
    GetHeaderSize () const;

    /**
     * \return whether or not this is actually a header or only a dummy (uninitialized) header
     */
    bool
    IsEmpty () const;

    // allow protocol-specific access to the header data.

    void
    SetXDistance (uint8_t xDistance);

    uint8_t
    const GetXDistance ();

    void
    SetYDistance (uint8_t yDistance);

    uint8_t
    const GetYDistance ();

    void
    SetSourceX (uint8_t sourceX);

    uint8_t
    const GetSourceX ();

    void
    SetSourceY (uint8_t sourceY);

    uint8_t
    const GetSourceY ();

    void
    SetSubdataId (uint8_t subdataId);

    uint8_t
    const GetSubdataId ();

    void
    SetPeGroupAddress (uint16_t peGroupAddress);

    uint16_t
    const GetPeGroupAddress ();

    void
    SetDataFlitCount (uint16_t dataFlitCount);

    uint16_t
    const GetDataFlitCount ();

    void
    SetLoad (uint8_t load);

    uint8_t
    const GetLoad ();

  private:

    /**
     * the size of this type of header, in bytes
     */
    // Make sure to update this field whenever you modify the structure of the header
    static const int HEADER_SIZE = 9;

    // m_xDistance and m_yDistance determine the destination address

    uint8_t m_xDistance; // this field is only 4 bits in size

    uint8_t m_yDistance; // this field is only 4 bits in size

    // m_sourceX and m_sourceY for the source address

    uint8_t m_sourceX; // this field is only 4 bits in size

    uint8_t m_sourceY; // this field is only 4 bits in size

    /**
     * allows package indexing; this is useful when data has to be split
     * into multiple packets (a package order is required is such a case)
     */
    uint8_t m_subdataId;

    /**
     * allows to specify a group of processing elements to which the packet is sent
     * (this allows multicasting)
     */
    uint16_t m_peGroupAddress;

    /**
     * Indicates how many data flits the packet contains;
     * this helps at determining the end of the packet
     *
     * Note that we consider a flit to have the size of a packet (to be a packet).
     */
    uint16_t m_dataFlitCount;

    /**
     * The load of a router (as a percentage number), propagated with the packet that has this header
     */
    uint8_t m_load;

  };

} // namespace ns3

#endif /* NOCHEADER_H_ */
