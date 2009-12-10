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

#ifndef WORMHOLESWITCHING_H_
#define WORMHOLESWITCHING_H_

#include "ns3/noc-switching-protocol.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  /**
   * \brief Wormhole flow control mechanism.
   *
   * \details
   * Wormhole switching is better than SAF switching in terms of both buffer
   * size and (unloaded) latency. The main drawback of WH switching is the
   * performance degradation due to a chain of packet blocks. Fractions of a packet
   * can be stored across different routers along the routing path in WH switching;
   * so a single packet often keeps occupying buffers in multiple routers along the
   * path, when the header of the packet cannot progress due to conflicts. Such
   * a situation is referred to as head-of-line (HOL) blocking. Buffers occupied
   * by the HOL blocking block other packets that want to go through the same
   * lines, resulting in performance degradation.
   *
   * (source: <a href = "http://www.amazon.com/Networks-Chips-Practice-Embedded-Multi-core/dp/1420079786">
   * Networks-on-Chips: Theory and Practice</a>)
   */
  class WormholeSwitching : public NocSwitchingProtocol
  {
  public:

    static TypeId
    GetTypeId();

    /**
     * Constructor
     */
    WormholeSwitching();

    virtual
    ~WormholeSwitching();

    virtual bool
    ApplyFlowControl(Ptr<Packet> packet, Ptr<Queue> bufferedPackets);

  private:

  };

} // namespace ns3

#endif /* WORMHOLESWITCHING_H_ */
