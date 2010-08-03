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

#ifndef NOCSWITCHINGPROTOCOL_H_
#define NOCSWITCHINGPROTOCOL_H_

#include "ns3/object.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-node.h"
#include "ns3/noc-net-device.h"
#include "ns3/queue.h"
#include <map>

namespace ns3
{

  class NocNetDevice;

  /**
   *
   * \brief Interface for the switching protocol used by NoC net devices
   *
   * Every switching protocol for NoCs must implement this interface.
   *
   */
  class NocSwitchingProtocol : public Object
  {
  public:

    static TypeId
    GetTypeId();

    NocSwitchingProtocol(std::string name);

    virtual
    ~NocSwitchingProtocol();

    /**
     * Perform flow control for the received packet
     *
     * \param packet            the packet to be resolved
     * \param bufferedPackets   the packets which are already buffered
     *
     * \return whether or not routing is allowed
     */
    virtual bool
    ApplyFlowControl (Ptr<Packet> packet, Ptr<Queue> bufferedPackets) = 0;

    /**
     * \return the name of this routing protocol
     */
    std::string
    GetName() const;

  private:

    /**
     * the name of the switching protocol
     */
    std::string m_name;

  };

} // namespace ns3

#endif /* NOCSWITCHINGPROTOCOL_H_ */
