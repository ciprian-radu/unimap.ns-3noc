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
   * \brief Header for the Network on Chip Protocol
   *
   * This class has fields corresponding to those in a Network on Chip (NoC) header
   * (destination address, source address, etc).
   */
  class NocHeader : public Header
  {
  public:
    NocHeader();

    NocHeader(uint8_t xDistance, uint8_t yDistance, uint8_t sourceX,
        uint8_t sourceY, uint16_t dataFlitCount);

    virtual
    ~NocHeader();

    static TypeId
    GetTypeId(void);

    virtual TypeId
    GetInstanceTypeId(void) const;

    virtual uint32_t
    GetSerializedSize(void) const;

    virtual void
    Serialize(Buffer::Iterator start) const;

    virtual uint32_t
    Deserialize(Buffer::Iterator start);

    virtual void
    Print(std::ostream &os) const;

    // allow protocol-specific access to the header data.

    void
    SetXDistance(uint8_t xDistance);

    uint8_t
    const GetXDistance();

    void
    SetYDistance(uint8_t yDistance);

    uint8_t
    const GetYDistance();

    void
    SetSourceX(uint8_t sourceX);

    uint8_t
    const GetSourceX();

    void
    SetSourceY(uint8_t sourceY);

    uint8_t
    const GetSourceY();

    void
    SetSubdataId(uint8_t subdataId);

    uint8_t
    const GetSubdataId();

    void
    SetPeGroupAddress(uint16_t peGroupAddress);

    uint16_t
    const GetPeGroupAddress();

    void
    SetDataFlitCount(uint16_t dataFlitCount);

    uint16_t
    const GetDataFlitCount();

  private:
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
     * indicates how many data flits the packet contains;
     * this helps at determining the end of the packet
     */
    uint16_t m_dataFlitCount;

  };

} // namespace ns3

#endif /* NOCHEADER_H_ */
