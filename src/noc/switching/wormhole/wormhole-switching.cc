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

#include "wormhole-switching.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"

NS_LOG_COMPONENT_DEFINE ("WormholeSwitching");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (WormholeSwitching);

  TypeId
  WormholeSwitching::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::WormholeSwitching")
        .SetParent<NocSwitchingProtocol> ()
        .AddConstructor<WormholeSwitching> ();
    return tid;
  }

  // we could easily name the protocol "Store and forward", but using __FILE__ should be more useful for debugging
  WormholeSwitching::WormholeSwitching() : NocSwitchingProtocol(__FILE__)
  {

  }

  WormholeSwitching::~WormholeSwitching()
  {

  }

  bool
  WormholeSwitching::ApplyFlowControl(Ptr<Packet> packet, Ptr<Queue> bufferedPackets)
  {
    // just leave any packet to pass, as soon at it arrives
    return true;
  }

} // namespace ns3
