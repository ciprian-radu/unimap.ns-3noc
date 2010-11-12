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
#include "ns3/boolean.h"
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
        .AddAttribute ("FullDuplex",
                       "The communication style of the channel may be half-duplex or full-duplex. By default full-duplex (i.e. true) is used.",
                       BooleanValue (true),
                       MakeBooleanAccessor (&NocChannel::m_fullDuplex),
                       MakeBooleanChecker ())
        .AddAttribute ("DataRate",
                       "The transmission data rate to be provided to devices connected to the channel",
                       DataRateValue (DataRate (0xffffffff)), // infinite data rate by default
                       MakeDataRateAccessor (&NocChannel::m_bps),
                       MakeDataRateChecker ())
        .AddAttribute ("Delay", "Transmission delay through the channel",
                       TimeValue (PicoSeconds (0)), // no channel delay by default
                       MakeTimeAccessor (&NocChannel::m_delay),
                       MakeTimeChecker ())
        ;
    return tid;
  }

  NocChannel::NocChannel () : Channel ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_fullDuplex = true;
    m_state.insert(m_state.end(), IDLE); // the state of the Tx line
    if (m_fullDuplex) {
        m_state.insert(m_state.end(), IDLE); // the state of the Rx line
    }
    m_currentPkt.insert(m_currentPkt.end(), 0);
    if (m_fullDuplex) {
        m_currentPkt.insert(m_currentPkt.end(), 0);
    }
    m_packetOriginalDevice.insert(m_packetOriginalDevice.end(), std::map<uint32_t, Ptr<NocNetDevice> > ());
    if (m_fullDuplex) {
        m_packetOriginalDevice.insert(m_packetOriginalDevice.end(), std::map<uint32_t, Ptr<NocNetDevice> > ());
    }
    m_currentDestDevice.insert(m_currentDestDevice.end(), 0);
    if (m_fullDuplex) {
        m_currentDestDevice.insert(m_currentDestDevice.end(), 0);
    }
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

    NS_ASSERT_MSG (m_devices.size () == 2, "A NoCchannel must be connected to two net devices!");
    NS_ASSERT (srcId < m_devices.size ());

    uint32_t link;
    if (!m_fullDuplex) {
        // note that "link" is used here only in full-duplex mode, to select one of the two physical links
        // hence, we set it to be zero for half-duplex
        link = 0;
    } else {
        link = srcId;
    }
    m_packetOriginalDevice[link].insert (std::pair<uint32_t, Ptr<NocNetDevice> > (p->GetUid (), originalNetDevice));

    if (m_state[link] != IDLE)
      {
        NS_LOG_WARN ("NoC channel state is not IDLE");
        return false;
      }
    m_currentPkt[link] = p;
    m_currentDestDevice[link] = 0;
    Ptr<NocNetDevice> sender = m_devices[srcId];
    for (std::vector<Ptr<NocNetDevice> >::const_iterator i = m_devices.begin(); i != m_devices.end(); ++i)
      {
        Ptr<NocNetDevice> tmp = *i;
        if (tmp != sender)
          {
            m_currentDestDevice[link] = tmp;
            break;
          }
      }
    NS_ASSERT (m_currentDestDevice[link] != 0);
    NS_LOG_DEBUG ("The receiving net device is " << m_currentDestDevice[link]->GetAddress ());
    if (m_currentDestDevice[link]->GetInQueueNPacktes () == m_currentDestDevice[link]->GetInQueueSize ())
      {
        NS_LOG_WARN ("The receiving net device " << m_currentDestDevice[link]->GetAddress ()
            << " will have no place to buffer the packet");
        return false;
      }

    m_state[link] = TRANSMITTING;
    if (!m_fullDuplex) {
        NS_LOG_LOGIC ("switched to TRANSMITTING");
    } else {
        NS_LOG_LOGIC ("switched to TRANSMITTING (for link " << (int)link << ")");
    }
    return true;
  }

  bool
  NocChannel::Send (Mac48Address to, Mac48Address from)
  {
    bool result = false;
    Ptr<NocNetDevice> sender = 0;
    uint32_t srcId;
    for (uint32_t i = 0; i < m_devices.size (); ++i) {
      if (from == Mac48Address::ConvertFrom(m_devices[i]->GetAddress())) {
          srcId = i;
          sender = m_devices[i];
          break;
      }
    }
    uint32_t link;
    if (!m_fullDuplex) {
        // note that "link" is used here only in full-duplex mode, to select one of the two physical links
        // hence, we set it to be zero for half-duplex
        link = 0;
    } else {
        link = srcId;
    }
    NS_ASSERT_MSG (sender != 0, "Could not identify the sender net device");
    NS_LOG_DEBUG ("Packet UID " << m_currentPkt[link]->GetUid ());
    NS_LOG_DEBUG ("Channel ID " << GetId ());
    NS_ASSERT (m_state[link] == TRANSMITTING);
    NS_LOG_DEBUG("number of devices for node " << sender->GetNode()->GetId() << " is " << m_devices.size());
    NS_LOG_DEBUG ("The packet " << (*m_currentPkt[link]) << " has size " << (int) m_currentPkt[link]->GetSize ());

    int speedup = 1;
    if (m_currentPkt[link] != 0)
      {
        NocHeader header;
        m_currentPkt[link]->PeekHeader (header);
        if (header.IsEmpty ())
          {
            // a data packet will be sent
            IntegerValue dataFlitSpeedup;
            Ptr<NocRegistry> nocRegistry = NocRegistry::GetInstance ();
            nocRegistry->GetAttribute ("DataPacketSpeedup", dataFlitSpeedup);
            speedup = dataFlitSpeedup.Get ();
          }
      }
    NS_LOG_LOGIC ("The channel has a delay of " << m_delay);
    NS_LOG_LOGIC ("Channel data rate (bandwidth) is " << m_bps);
    if (speedup != 1)
      {
        NS_LOG_LOGIC ("Data flit speedup is " << speedup);
      }
    else
      {
        NS_LOG_DEBUG ("Data flit speedup is " << speedup);
      }

    if (m_fullDuplex) {
        NS_LOG_LOGIC ("Working in full-duplex mode");
    } else {
        NS_LOG_LOGIC ("Working in half-duplex mode");
    }
    Time tEvent = Seconds (m_bps.CalculateTxTime (m_currentPkt[link]->GetSize ()));
    NS_LOG_LOGIC ("The channel will send the packet in " << m_delay + tEvent
        << " (" << m_delay << " + " << tEvent
        << ") from " << from << " to " << m_currentDestDevice[link]->GetAddress () << " (final destination is " << to << ")");
    m_state[link] = PROPAGATING;
    if (!m_fullDuplex) {
        NS_LOG_LOGIC ("switched to PROPAGATING");
    } else {
        NS_LOG_LOGIC ("switched to PROPAGATING (for link " << (int)link << ")");
    }
    NS_LOG_DEBUG ("Schedule event (channel transmission) to occur at time "
        << Simulator::Now() + (m_delay + tEvent) / Scalar (speedup));
    // if default values for delay and data rate are used (0 and respectively infinite), then the channel transports the data instantly
    Simulator::Schedule ((m_delay + tEvent) / Scalar (speedup), &NocChannel::TransmitEnd, this, sender, to,
        m_currentDestDevice[link], from);

    result = true;

    return result;
  }

  void
  NocChannel::TransmitEnd (Ptr<NocNetDevice> srcNocNetDevice, Mac48Address to, Ptr<NocNetDevice> destNocNetDevice, Mac48Address from)
  {
    uint32_t srcId;
    bool found = false;
    for (uint32_t i = 0; i < m_devices.size (); ++i) {
      if (srcNocNetDevice == m_devices[i]) {
          srcId = i;
          found = true;
          break;
      }
    }
    NS_ASSERT_MSG (found == true, "Could not identify the sender net device");
    uint32_t link;
    if (!m_fullDuplex) {
        // note that "link" is used here only in full-duplex mode, to select one of the two physical links
        // hence, we set it to be zero for half-duplex
        link = 0;
    } else {
        link = srcId;
    }
    NS_LOG_FUNCTION (*m_currentPkt[link]);
    NS_LOG_DEBUG ("Packet UID " << m_currentPkt[link]->GetUid ());
    NS_LOG_DEBUG ("Channel ID " << GetId ());
    NS_ASSERT (m_state[link] == PROPAGATING);

    Ptr<NocRouter> router = destNocNetDevice->GetNode ()->GetObject<NocNode> ()->GetRouter ();
    Ptr<LoadRouterComponent> loadComponent = router->GetLoadRouterComponent ();
    if (loadComponent != 0)
      {
        NS_LOG_DEBUG ("Load component found");

        loadComponent->IncreaseLoad ();
        NocHeader nocHeader;
        m_currentPkt[link]->PeekHeader (nocHeader);
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

    Ptr<NocNetDevice> originalNetDevice = m_packetOriginalDevice[link][m_currentPkt[link]->GetUid ()];
    NS_ASSERT (originalNetDevice != 0);
    NS_LOG_LOGIC ("Dequeuing the transmitted packet from the queue of net device "
        << originalNetDevice->GetAddress ());
    Ptr<const Packet> dequeuedPacket = originalNetDevice->DequeuePacketFromInQueue ();
    NS_ASSERT (dequeuedPacket != 0);
    NS_ASSERT_MSG(m_currentPkt[link]->GetUid () == dequeuedPacket->GetUid (),
        "The transmitted packet (UID " << m_currentPkt[link]->GetUid ()
        << ") should have been dequeued, not packet with UID " << dequeuedPacket->GetUid ());
    m_packetOriginalDevice[link].erase (m_currentPkt[link]->GetUid ());

    NS_LOG_LOGIC ("The channel is calling the Receive method of the net device");
    destNocNetDevice->Receive (m_currentPkt[link]->Copy (), to, from);

    m_state[link] = IDLE;
    if (!m_fullDuplex) {
        NS_LOG_LOGIC ("switched to IDLE");
    } else {
        NS_LOG_LOGIC ("switched to IDLE (for link " << (int)link << ")");
    }
  }

  uint32_t
  NocChannel::Add (Ptr<NocNetDevice> device)
  {
    m_devices.push_back (device);
    NS_ASSERT_MSG (m_devices.size () <= 2, "A NocChannel allows two devices to be connected to it!");
    return (m_devices.size () - 1);
  }

  uint32_t
  NocChannel::GetNDevices () const
  {
    NS_ASSERT_MSG (m_devices.size () <= 2, "A NocChannel allows two devices to be connected to it!");
    return m_devices.size ();
  }

  Ptr<NetDevice>
  NocChannel::GetDevice (uint32_t i) const
  {
    return m_devices[i];
  }

  bool
  NocChannel::IsBusy (Ptr<NocNetDevice> device)
  {
    uint32_t srcId;
    bool found = false;
    for (uint32_t i = 0; i < m_devices.size (); ++i) {
      if (device == m_devices[i]) {
          srcId = i;
          found = true;
          break;
      }
    }
    NS_ASSERT_MSG (found == true, "Could not identify the sender net device");
    uint32_t link;
    if (!m_fullDuplex) {
        // note that "link" is used here only in full-duplex mode, to select one of the two physical links
        // hence, we set it to be zero for half-duplex
        link = 0;
    } else {
        link = srcId;
    }
    if (m_state[link] == IDLE)
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
  NocChannel::GetState (Ptr<NocNetDevice> device)
  {
    uint32_t srcId;
    bool found = false;
    for (uint32_t i = 0; i < m_devices.size (); ++i) {
      if (device == m_devices[i]) {
          srcId = i;
          found = true;
          break;
      }
    }
    NS_ASSERT_MSG (found == true, "Could not identify the sender net device");
    uint32_t link;
    if (!m_fullDuplex) {
        // note that "link" is used here only in full-duplex mode, to select one of the two physical links
        // hence, we set it to be zero for half-duplex
        link = 0;
    } else {
        link = srcId;
    }
    return m_state[link];
  }

} // namespace ns3
