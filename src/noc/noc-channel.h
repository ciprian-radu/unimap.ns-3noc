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

#ifndef NOCCHANNEL_H_
#define NOCCHANNEL_H_

#include "ns3/channel.h"
#include "ns3/mac48-address.h"
#include <vector>
#include "noc-packet.h"
#include "noc-net-device.h"

namespace ns3
{

  class NocNetDevice;

  /**
   * \ingroup channel
   * \brief A channel for a Network on Chip (NoC)
   */
  class NocChannel : public Channel
  {
  public:
    static TypeId
    GetTypeId(void);
    NocChannel();

    void
    Send(Ptr<Packet> p, uint16_t protocol, Mac48Address to,
        Mac48Address from, Ptr<NocNetDevice> sender);

    void
    Add(Ptr<NocNetDevice> device);

    // inherited from ns3::Channel
    virtual uint32_t
    GetNDevices(void) const;
    virtual Ptr<NetDevice>
    GetDevice(uint32_t i) const;

  private:
    std::vector<Ptr<NocNetDevice> > m_devices;
  };

} // namespace ns3

#endif /* NOCCHANNEL_H_ */
