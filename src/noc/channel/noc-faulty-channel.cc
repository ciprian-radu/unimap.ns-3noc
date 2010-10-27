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

#include "noc-faulty-channel.h"
#include "ns3/noc-net-device.h"
#include "ns3/noc-packet.h"
#include "ns3/log.h"
#include "ns3/noc-node.h"
#include "ns3/noc-header.h"
#include "ns3/boolean.h"
#include "ns3/noc-registry.h"

NS_LOG_COMPONENT_DEFINE ("NocFaultyChannel");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocFaultyChannel);

  TypeId
  NocFaultyChannel::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::NocFaultyChannel")
        .SetParent<Channel> ()
        .AddConstructor<NocFaultyChannel> ()
        .AddAttribute (
            "Faulty",
            "Indicates if a router is faulty or not",
            BooleanValue (false),
            MakeBooleanAccessor (&NocFaultyChannel::m_faulty),
            MakeBooleanChecker ())
        ;
    return tid;
  }

  NocFaultyChannel::NocFaultyChannel () : NocChannel ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  bool
  NocFaultyChannel::TransmitStart (Ptr<NocNetDevice> originalNetDevice, Ptr<Packet> p, uint32_t srcId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    bool start = false;

    if (!m_faulty)
      {
        start = NocChannel::TransmitStart (originalNetDevice, p, srcId);
      }
    else
      {
        NS_LOG_ERROR ("Cannot start transmitting because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return start;
  }

  bool
  NocFaultyChannel::Send (Mac48Address to, Mac48Address from)
  {
    NS_LOG_FUNCTION_NOARGS ();

    bool result = false;

    if (!m_faulty)
      {
        result = NocChannel::Send (to, from);
      }
    else
      {
        NS_LOG_ERROR ("Cannot start sending because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return result;
  }

  void
  NocFaultyChannel::TransmitEnd (Ptr<NocNetDevice> srcNocNetDevice, Mac48Address to, Ptr<NocNetDevice> destNocNetDevice, Mac48Address from)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocChannel::TransmitEnd (srcNocNetDevice, to, destNocNetDevice, from);
      }
    else
      {
        NS_LOG_ERROR ("Cannot end transmitting because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  uint32_t
  NocFaultyChannel::Add (Ptr<NocNetDevice> device)
  {
    NS_LOG_FUNCTION_NOARGS ();

    uint32_t index;

    if (!m_faulty)
      {
        index = NocChannel::Add (device);
      }
    else
      {
        NS_LOG_ERROR ("Cannot add device because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return index;
  }

  uint32_t
  NocFaultyChannel::GetNDevices () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    uint32_t n;

    if (!m_faulty)
      {
        n = NocChannel::GetNDevices ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get the number of devices because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return n;
  }

  Ptr<NetDevice>
  NocFaultyChannel::GetDevice (uint32_t i) const
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NetDevice> device;

    if (!m_faulty)
      {
        device = NocChannel::GetDevice (i);
      }
    else
      {
        NS_LOG_ERROR ("Cannot get device because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return device;
  }

  bool
  NocFaultyChannel::IsBusy (Ptr<NocNetDevice> device)
  {
    NS_LOG_FUNCTION_NOARGS ();

    bool busy = false;

    if (!m_faulty)
      {
        busy = NocChannel::IsBusy (device);
      }
    else
      {
        NS_LOG_ERROR ("Cannot check busy state because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return busy;
  }

  NocChannel::WireState
  NocFaultyChannel::GetState (Ptr<NocNetDevice> device)
  {
    NS_LOG_FUNCTION_NOARGS ();

    NocChannel::WireState wireState;

    if (!m_faulty)
      {
      wireState = NocChannel::GetState (device);
      }
    else
      {
        NS_LOG_ERROR ("Cannot get wire state because channel " << (int) GetId ()
            << " is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return wireState;
  }

} // namespace ns3
