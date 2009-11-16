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

#include "noc-channel.h"
#include "noc-net-device.h"
#include "noc-packet.h"
#include "ns3/simulator.h"

namespace ns3
{

  TypeId
  NocChannel::GetTypeId(void)
  {
    static TypeId tid =
        TypeId("ns3::NocChannel") .SetParent<Channel> () .AddConstructor<
            NocChannel> ();
    return tid;
  }

  NocChannel::NocChannel()
  {
  }

  void
  NocChannel::Send(Ptr<Packet> p, uint16_t protocol, Mac48Address to,
      Mac48Address from, Ptr<NocNetDevice> sender)
  {
    for (std::vector<Ptr<NocNetDevice> >::const_iterator i = m_devices.begin(); i
        != m_devices.end(); ++i)
      {
        Ptr<NocNetDevice> tmp = *i;
        if (tmp == sender)
          {
            continue;
          }
        Simulator::ScheduleNow(&NocNetDevice::Receive, tmp, p->Copy(),
            protocol, to, from);
      }
  }

  void
  NocChannel::Add(Ptr<NocNetDevice> device)
  {
    m_devices.push_back(device);
  }

  uint32_t
  NocChannel::GetNDevices(void) const
  {
    return m_devices.size();
  }

  Ptr<NetDevice>
  NocChannel::GetDevice(uint32_t i) const
  {
    return m_devices[i];
  }

} // namespace ns3
