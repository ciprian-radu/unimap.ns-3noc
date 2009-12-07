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

#ifndef NOCPACKET_H_
#define NOCPACKET_H_

#include "ns3/packet.h"
#include "ns3/log.h"

namespace ns3
{

  class NocPacket : public Packet
  {
  public:

    /**
     * Constructor - creates a head packet
     *
     * \param xDistance the X (horizontal) offset to the destination
     * \param yDistance the Y (vertical) offset to the destination
     * \param sourceX the X coordinate of the source node
     * \param sourceY the Y coordinate of the source node
     * \param dataFlitCount the number of data flits (packets) that the message having this header will have
     * \param dataPacketSize the size of the head packet (without including the size of the header)
     */
    NocPacket (uint8_t xDistance, uint8_t yDistance, uint8_t sourceX,
        uint8_t sourceY, uint16_t dataFlitCount, uint32_t dataPacketSize);

    /**
     * Constructor - creates a data packet
     *
     * \param dataPacketSize the size of the head packet (without including the size of the header)
     */
    NocPacket (uint32_t dataPacketSize);

    ~NocPacket ();

    /**
     * \return whether or not this packet is a head packet or not (i.e. contains a header)
     */
    bool
    IsHeadPacket ();

    /**
     * \see IsHeadPacket ()
     *
     * \return whether or not this packet is a data packet or not (i.e. does not contain a header)
     */
    bool
    IsDataPacket ();

  private:

    bool m_isHeadPacket;
  };

  std::ostream& operator<< (std::ostream& os, NocPacket &packet);

  std::ostream& operator<< (std::ostream& os, Ptr<NocPacket> packet);

} // namespace ns3

#endif /* NOCPACKET_H_ */
