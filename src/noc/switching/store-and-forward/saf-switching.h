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

#ifndef SAFSWITCHING_H_
#define SAFSWITCHING_H_

#include "ns3/noc-switching-protocol.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  /**
   * \brief Store and forward flow control mechanism.
   *
   * \details
   * Every packet is split into transfer units called flits (note that we consider
   * a packet to be a flit here; multiple packets for a message). A single flit is sent
   * from an output port of a router at each time unit. Once a router receives a header
   * flit, the body flits of the packet arrive every time unit. To simply avoid input-
   * channel buffer overflow, the input buffer must be larger than the maximum
   * packet size. The header flit is forwarded to the neighboring router after it
   * receives the tail flit. This switching technique is called store-and-forward (SAF).
   * The advantage of SAF switching is the simple needed control mechanism
   * between routers due to packet-based operation.
   * The main drawback of SAF switching is the large needed channel buffer size that
   * increases the hardware amount of the router. Moreover,
   * SAF suffers from a larger latency compared with other switching techniques,
   * because a router in every hop must wait to receive the entire packet before
   * forwarding the header flit. Thus, SAF switching does not fit well with the
   * requirements of NoCs.
   *
   * (source: <a href = "http://www.amazon.com/Networks-Chips-Practice-Embedded-Multi-core/dp/1420079786">
   * Networks-on-Chips: Theory and Practice</a>)
   *
   */
  class SafSwitching : public NocSwitchingProtocol
  {
  public:

    static TypeId
    GetTypeId();

    /**
     * Constructor
     */
    SafSwitching();

    virtual
    ~SafSwitching();

    virtual bool
    ApplyFlowControl(Ptr<Packet> packet, Ptr<Queue> bufferedPackets);

  private:

    /**
     * for each packet UID count the number of received data (body) flits
     *
     * the counter will start from the required value and will decrease until reaching zero
     */
    std::map<uint32_t , uint32_t> m_flitCount;

  };

} // namespace ns3

#endif /* SAFSWITCHING_H_ */
