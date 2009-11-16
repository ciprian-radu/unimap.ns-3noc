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
    NocPacket(uint8_t xDistance, uint8_t yDistance, uint8_t sourceX,
        uint8_t sourceY, uint16_t dataFlitCount);

  private:
  };

} // namespace ns3

#endif /* NOCPACKET_H_ */
