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
#include "ns3/noc-net-device.h"
#include "ns3/noc-packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("NocChannel");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocChannel);

  TypeId
  NocChannel::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::NocChannel")
        .SetParent<Channel> ()
        .AddConstructor<NocChannel> ()
        .AddAttribute ("DataRate",
                       "The transmission data rate to be provided to devices connected to the channel",
                       DataRateValue (DataRate (0xffffffff)), // infinite data rate by default
                       MakeDataRateAccessor (&NocChannel::m_bps),
                       MakeDataRateChecker ())
        .AddAttribute ("Delay", "Transmission delay through the channel",
                       TimeValue (Seconds (0)), // no channel delay by default
                       MakeTimeAccessor (&NocChannel::m_delay),
                       MakeTimeChecker ())
        ;
    return tid;
  }

  NocChannel::NocChannel() : Channel ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_state = IDLE;
    NS_LOG_LOGIC ("switched to IDLE");
    m_devices.clear();
  }

  bool
  NocChannel::TransmitStart(Ptr<Packet> p, uint32_t srcId)
  {
    NS_LOG_FUNCTION (p << srcId);
    NS_LOG_INFO ("Packet UID is " << p->GetUid ());

    if (m_state != IDLE)
      {
        NS_LOG_WARN ("NoC channel state is not IDLE");
        return false;
      }

    m_currentPkt = p;
    m_currentSrc = srcId;
    m_state = TRANSMITTING;
    NS_LOG_LOGIC ("switched to TRANSMITTING");
    return true;
  }

  bool
  NocChannel::Send(Mac48Address to, Mac48Address from)
  {
    bool result = false;
    NS_LOG_FUNCTION (this << m_currentPkt << m_currentSrc);
    NS_LOG_INFO ("Packet UID is " << m_currentPkt->GetUid ());
    NS_ASSERT (m_state == TRANSMITTING);
      Ptr<NocNetDevice> sender = m_devices[m_currentSrc];
      NS_LOG_DEBUG("number of devices for node " << sender->GetNode()->GetId() << " is " << m_devices.size());
      for (std::vector<Ptr<NocNetDevice> >::const_iterator i = m_devices.begin(); i != m_devices.end(); ++i)
        {
          Ptr<NocNetDevice> tmp = *i;
          if (tmp != sender)
            {
              Time tEvent = Seconds (m_bps.CalculateTxTime (m_currentPkt->GetSize ()));
              NS_LOG_LOGIC ("The channel will send the packet in " << (m_delay.GetSeconds () + tEvent.GetSeconds())
                  << " seconds (" << m_delay.GetSeconds () << " + " << tEvent.GetSeconds() << ")");
              m_state = PROPAGATING;
              NS_LOG_LOGIC ("switched to PROPAGATING");
              Simulator::Schedule(m_delay + tEvent, &NocChannel::TransmitEnd, this, to, tmp, from);
              result = true;
            }
        }
    return result;
  }

  void
  NocChannel::TransmitEnd(Mac48Address to, Ptr<NocNetDevice> destNocNetDevice, Mac48Address from)
  {
    NS_LOG_FUNCTION (m_currentPkt);
    NS_LOG_INFO ("Packet UID is " << m_currentPkt->GetUid ());
    NS_ASSERT (m_state == PROPAGATING);
    NS_LOG_LOGIC ("The channel is calling the Receive method of the net device");

    destNocNetDevice->Receive (m_currentPkt->Copy (), to, from);
    m_state = IDLE;
    NS_LOG_LOGIC ("switched to IDLE");
  }

  uint32_t
  NocChannel::Add(Ptr<NocNetDevice> device)
  {
    m_devices.push_back(device);
    return (m_devices.size () - 1);
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

  bool
  NocChannel::IsBusy(void)
  {
    if (m_state == IDLE)
      {
        return false;
      }
    else
      {
        return true;
      }
  }

  DataRate
  NocChannel::GetDataRate(void)
  {
    return m_bps;
  }

  Time
  NocChannel::GetDelay(void)
  {
    return m_delay;
  }

  NocChannel::WireState
  NocChannel::GetState(void)
  {
    return m_state;
  }

} // namespace ns3
