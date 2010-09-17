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
#include "ns3/noc-node.h"
#include "ns3/noc-header.h"
#include "ns3/integer.h"
#include "ns3/noc-registry.h"

NS_LOG_COMPONENT_DEFINE ("NocChannel");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocChannel);

  TypeId
  NocChannel::GetTypeId ()
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

  NocChannel::NocChannel () : Channel ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_state = IDLE;
    NS_LOG_LOGIC ("switched to IDLE");
    m_devices.clear();
  }

  bool
  NocChannel::TransmitStart (Ptr<NocNetDevice> originalNetDevice, Ptr<Packet> p, uint32_t srcId)
  {
    NS_LOG_FUNCTION ("original net device" << originalNetDevice->GetAddress () << "packet" << *p << "node" << (int) srcId);
    NS_LOG_DEBUG ("Packet UID " << p->GetUid ());
    NS_LOG_DEBUG ("Channel ID " << GetId ());
    NS_LOG_DEBUG ("Original net device " << originalNetDevice->GetAddress ());

    m_packetOriginalDevice.insert (std::pair<uint32_t, Ptr<NocNetDevice> > (p->GetUid (), originalNetDevice));

    if (m_state != IDLE)
      {
        NS_LOG_WARN ("NoC channel state is not IDLE");
        return false;
      }
    m_currentPkt = p;
    m_currentSrc = srcId;
    m_currentDestDevice = 0;
    Ptr<NocNetDevice> sender = m_devices[m_currentSrc];
    for (std::vector<Ptr<NocNetDevice> >::const_iterator i = m_devices.begin(); i != m_devices.end(); ++i)
      {
        Ptr<NocNetDevice> tmp = *i;
        if (tmp != sender)
          {
            m_currentDestDevice = tmp;
            break;
          }
      }
    NS_ASSERT (m_currentDestDevice != 0);
    NS_LOG_DEBUG ("The receiving net device is " << m_currentDestDevice->GetAddress ());
    if (m_currentDestDevice->GetInQueueNPacktes () == m_currentDestDevice->GetInQueueSize ())
      {
        NS_LOG_WARN ("The receiving net device " << m_currentDestDevice->GetAddress ()
            << " will have no place to buffer the packet");
        return false;
      }

    m_state = TRANSMITTING;
    NS_LOG_LOGIC ("switched to TRANSMITTING");
    return true;
  }

  bool
  NocChannel::Send (Mac48Address to, Mac48Address from)
  {
    bool result = false;
    NS_LOG_DEBUG ("Packet UID " << m_currentPkt->GetUid ());
    NS_LOG_DEBUG ("Channel ID " << GetId ());
    NS_ASSERT (m_state == TRANSMITTING);
    Ptr<NocNetDevice> sender = m_devices[m_currentSrc];
    NS_LOG_DEBUG("number of devices for node " << sender->GetNode()->GetId() << " is " << m_devices.size());
    NS_LOG_DEBUG ("The packet " << (*m_currentPkt) << " has size " << (int) m_currentPkt->GetSize ());

    TimeValue timeValue;
    NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
    Time globalClock = timeValue.Get ();
    if (globalClock.IsZero ())
      {
        Time tEvent = Seconds (m_bps.CalculateTxTime (m_currentPkt->GetSize ()));
        NS_LOG_LOGIC ("The channel will send the packet in " << (m_delay.GetSeconds () + tEvent.GetSeconds())
            << " seconds (" << m_delay.GetSeconds () << " + " << tEvent.GetSeconds()
            << ") from " << from << " to " << m_currentDestDevice->GetAddress () << " (final destination is " << to << ")");
        m_state = PROPAGATING;
        NS_LOG_LOGIC ("switched to PROPAGATING");
        NS_LOG_DEBUG ("Schedule event (net device receive) to occur at time "
            << (Simulator::Now() + m_delay + tEvent).GetSeconds () << " seconds");
        Simulator::Schedule(m_delay + tEvent, &NocChannel::TransmitEnd, this, sender, to, m_currentDestDevice, from);
      }
    else
      {
        int speedup = 1;
        if (m_currentPkt != 0)
          {
            NocHeader header;
            m_currentPkt->PeekHeader (header);
            if (header.IsEmpty ())
              {
                // a data packet will be sent
                IntegerValue dataFlitSpeedup;
                Ptr<NocRegistry> nocRegistry = NocRegistry::GetInstance ();
                nocRegistry->GetAttribute ("DataPacketSpeedup", dataFlitSpeedup);
                speedup = dataFlitSpeedup.Get ();
              }
          }
        NS_LOG_DEBUG ("Data flit speedup is " << speedup);

        NS_LOG_LOGIC ("The channel will send the packet in " << (globalClock / Scalar (speedup)).GetSeconds()
            << " seconds from " << from << " to " << m_currentDestDevice->GetAddress () << " (final destination is " << to << ")");
        m_state = PROPAGATING;
        NS_LOG_LOGIC ("switched to PROPAGATING");
        NS_LOG_DEBUG ("Schedule event (net device receive) to occur at time "
            << (Simulator::Now() + globalClock / Scalar (speedup)).GetSeconds () << " seconds");
        Simulator::Schedule(globalClock / Scalar (speedup), &NocChannel::TransmitEnd, this, sender, to, m_currentDestDevice, from);
      }
    result = true;

    return result;
  }

  void
  NocChannel::TransmitEnd (Ptr<NocNetDevice> srcNocNetDevice, Mac48Address to, Ptr<NocNetDevice> destNocNetDevice, Mac48Address from)
  {
    NS_LOG_FUNCTION (*m_currentPkt);
    NS_LOG_DEBUG ("Packet UID " << m_currentPkt->GetUid ());
    NS_LOG_DEBUG ("Channel ID " << GetId ());
    NS_ASSERT (m_state == PROPAGATING);

    Ptr<NocRouter> router = destNocNetDevice->GetNode ()->GetObject<NocNode> ()->GetRouter ();
    Ptr<LoadRouterComponent> loadComponent = router->GetLoadRouterComponent ();
    if (loadComponent != 0)
      {
        NS_LOG_DEBUG ("Load component found");

        loadComponent->IncreaseLoad ();
        NocHeader nocHeader;
        m_currentPkt->PeekHeader (nocHeader);
        if (!nocHeader.IsEmpty ())
          {
            uint8_t load = nocHeader.GetLoad ();
            router->AddNeighborLoad ((int) load, srcNocNetDevice);
          }
      }
    else
      {
        NS_LOG_DEBUG ("No load component found");
      }

    Ptr<NocNetDevice> originalNetDevice = m_packetOriginalDevice[m_currentPkt->GetUid ()];
    NS_ASSERT (originalNetDevice != 0);
    NS_LOG_LOGIC ("Dequeuing the transmitted packet from the queue of net device "
        << originalNetDevice->GetAddress ());
    Ptr<const Packet> dequeuedPacket = originalNetDevice->DequeuePacketFromInQueue ();
    NS_ASSERT (dequeuedPacket != 0);
    NS_ASSERT_MSG(m_currentPkt->GetUid () == dequeuedPacket->GetUid (),
        "The transmitted packet (UID " << m_currentPkt->GetUid ()
        << ") should have been dequeued, not packet with UID " << dequeuedPacket->GetUid ());
    m_packetOriginalDevice.erase (m_currentPkt->GetUid ());

    NS_LOG_LOGIC ("The channel is calling the Receive method of the net device");
    destNocNetDevice->Receive (m_currentPkt->Copy (), to, from);

    m_state = IDLE;
    NS_LOG_LOGIC ("switched to IDLE");
  }

  uint32_t
  NocChannel::Add (Ptr<NocNetDevice> device)
  {
    m_devices.push_back (device);
    return (m_devices.size () - 1);
  }

  uint32_t
  NocChannel::GetNDevices () const
  {
    return m_devices.size ();
  }

  Ptr<NetDevice>
  NocChannel::GetDevice (uint32_t i) const
  {
    return m_devices[i];
  }

  bool
  NocChannel::IsBusy ()
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
  NocChannel::GetDataRate ()
  {
    return m_bps;
  }

  Time
  NocChannel::GetDelay ()
  {
    return m_delay;
  }

  NocChannel::WireState
  NocChannel::GetState ()
  {
    return m_state;
  }

} // namespace ns3
