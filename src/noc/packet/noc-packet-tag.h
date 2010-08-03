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

#include "ns3/noc-packet.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/nstime.h"
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
     * \param type the type of the packet
     */
    void
    SetPacketType (NocPacket::Type type);

    /**
     * \return the type of the packet
     */
    NocPacket::Type
    GetPacketType () const;

    /**
     * \param uid the UID of the head packet
     */
    void
    SetPacketHeadUid (uint32_t uid);

    /**
     * \return the UID of the packet head
     */
    uint32_t
    GetPacketHeadUid () const;

    /**
     * \param isBlocked whether or not this packet is blocked
     */
    void
    SetPacketBlocked (bool isBlocked);

    /**
     * \return whether or not this packet is blocked
     */
    bool
    GetPacketBlocked () const;

    /**
     * \param injectionTime the time of injection
     */
    void
    SetInjectionTime (Time injectionTime);

    /**
     * \return the injection time
     */
    Time
    GetInjectionTime () const;

    /**
     * \param receiveTime the time when the packet was received
     */
    void
    SetReceiveTime (Time receiveTime);

    /**
     * \return the receive time
     */
    Time
    GetReceiveTime () const;

  private:

    /**
     * the type of the packet
     */
    NocPacket::Type m_type;

    /**
     * the unique identifier of the head packet
     * (this is useful for data packets)
     */
    uint32_t m_headPacketUid;

    /**
     * whether or not a head packet is blocked in the network
     */
    bool m_packetBlocked;

    /**
     * at what (simulation) time the packet was injected into the network
     */
    Time m_injectionTime;

    /**
     * at what (simulation) time the packet was received by the destination node
     */
    Time m_receiveTime;
  };

} // namespace ns3

#endif /* NOCPACKETTAG_H_ */
