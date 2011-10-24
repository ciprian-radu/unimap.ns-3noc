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

#ifndef VCTSWITCHING_H_
#define VCTSWITCHING_H_

#include "ns3/noc-switching-protocol.h"
#include "ns3/noc-net-device.h"
#include <set>

namespace ns3
{

  /**
   * \brief Virtual cut-through flow control mechanism.
   *
   * \details
   *
   * (source: <a href = "http://www.amazon.com/Networks-Chips-Practice-Embedded-Multi-core/dp/1420079786">
   * Networks-on-Chips: Theory and Practice</a>)
   *
   */
  class VctSwitching : public NocSwitchingProtocol
  {
  public:

    static TypeId
    GetTypeId();

    /**
     * Constructor
     */
    VctSwitching();

    virtual
    ~VctSwitching();

    virtual bool
    ApplyFlowControl(Ptr<Packet> packet, Ptr<Queue> bufferedPackets);

  private:

    /**
     * for each packet UID count the number of received data (body) flits
     *
     * the counter will start from the required value and will decrease until reaching zero
     */
    std::map<uint32_t , uint32_t> m_flitCount;

    /**
     * keeps all the head flits which are blocked
     */
    std::set<uint32_t> m_blockedHeadFlits;

  };

} // namespace ns3

#endif /* VCTSWITCHING_H_ */
