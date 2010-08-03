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

#include "ns3/log.h"
#include "route.h"

NS_LOG_COMPONENT_DEFINE ("Route");

namespace ns3
{

  TypeId
  Route::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::Route")
        .SetParent<Object> ();
    return tid;
  }

  Route::Route (Ptr<Packet> routedPacket, Ptr<NocNetDevice> sourceDevice, Ptr<NocNetDevice> destinationDevice)
  {
    m_routedPacket = routedPacket;
    m_sourceDevice = sourceDevice;
    m_destinationDevice = destinationDevice;
  }

  Route::~Route ()
  {
    ;
  }

  Ptr<Packet>
  Route::GetRoutedPacket () const
  {
    return m_routedPacket;
  }

  Ptr<NocNetDevice>
  Route::GetSourceDevice () const
  {
    return m_sourceDevice;
  }

  Ptr<NocNetDevice>
  Route::GetDestinationDevice () const
  {
    return m_destinationDevice;
  }

}  // namespace ns3
