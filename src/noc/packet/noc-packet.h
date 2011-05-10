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

    // the order of these values is considered in NocPacketTag
    // (do not change it without making changes there)
    enum Type {HEAD, DATA, TAIL, UNKNOWN};

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
     * Constructor - creates a head packet
     *
     * \param xDistance the X (horizontal) offset to the destination
     * \param yDistance the Y (vertical) offset to the destination
     * \param yDistance the Z offset to the destination
     * \param sourceX the X coordinate of the source node
     * \param sourceY the Y coordinate of the source node
     * \param sourceZ the Z coordinate of the source node
     * \param dataFlitCount the number of data flits (packets) that the message having this header will have
     * \param dataPacketSize the size of the head packet (without including the size of the header)
     */
    NocPacket (uint8_t xDistance, uint8_t yDistance, uint8_t zDistance, uint8_t sourceX,
        uint8_t sourceY, uint8_t sourceZ, uint16_t dataFlitCount, uint32_t dataPacketSize);

    /**
     * Constructor - creates a head packet
     *
     * \param distance the  offsets to the destination
     * \param source the coordinates of the source node
     * \param dataFlitCount the number of data flits (packets) that the message having this header will have
     * \param dataPacketSize the size of the head packet (without including the size of the header)
     */
    NocPacket (std::vector <uint8_t> distance, std::vector<uint8_t> source, uint16_t dataFlitCount,
        uint32_t dataPacketSize);

    /**
     * Constructor - creates a data packet
     *
     * \param headPacketUid the UID of the head packet
     * \param dataPacketSize the size of the data packet
     * \param isTailPacket wheter or not this is the last data packet from the message (i.e. the tail)
     */
    NocPacket (uint32_t headPacketUid, uint32_t dataPacketSize, bool isTailPacket);

    ~NocPacket ();

  private:

  };

} // namespace ns3

#endif /* NOCPACKET_H_ */
