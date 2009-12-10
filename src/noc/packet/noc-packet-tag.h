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

#ifndef NOCPACKETTAG_H_
#define NOCPACKETTAG_H_

#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include <iostream>

namespace ns3
{

  class NocPacketTag : public Tag
  {
  public:

    NocPacketTag ();

    static TypeId
    GetTypeId ();

    virtual TypeId
    GetInstanceTypeId () const;

    virtual uint32_t
    GetSerializedSize () const;

    virtual void
    Serialize (TagBuffer i) const;

    virtual void
    Deserialize (TagBuffer i);

    virtual void
    Print (std::ostream &os) const;

    /**
     * \param uid the UID of the head packet
     */
    void
    SetPacketHeadUid (uint8_t uid);

    /**
     * \return the UID of the packet head
     */
    uint8_t
    GetPacketHeadUid () const;

    /**
     * \param whether or not this packet is blocked
     */
    void
    SetPacketBlocked (bool isBlocked);

    /**
     * \return whether or not this packet is blocked
     */
    bool
    GetPacketBlocked () const;

  private:

    uint8_t m_headPacketUid;

    bool m_packetBlocked;
  };

} // namespace ns3

#endif /* NOCPACKETTAG_H_ */
