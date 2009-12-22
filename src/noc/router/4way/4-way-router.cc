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

#include "4-way-router.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"

NS_LOG_COMPONENT_DEFINE ("FourWayRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (FourWayRouter);

  TypeId
  FourWayRouter::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::FourWayRouter")
        .SetParent<NocRouter> ();
    return tid;
  }

  // we could easily name the router "four way router", but using __FILE__ should be more useful for debugging
  FourWayRouter::FourWayRouter () : NocRouter (__FILE__)
  {
    ;
  }

  FourWayRouter::FourWayRouter (std::string name) : NocRouter (name)
  {
    ;
  }


  FourWayRouter::~FourWayRouter ()
  {
    ;
  }

  Ptr<NocNetDevice>
  FourWayRouter::GetInjectionNetDevice (Ptr<NocPacket> packet, Ptr<NocNode> destination)
  {
    NS_LOG_FUNCTION_NOARGS ();
    return GetNocNode ()->GetDevice (0)->GetObject<NocNetDevice> ();
  }

  Ptr<NocNetDevice>
  FourWayRouter::GetInputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection)
  {
    NS_LOG_DEBUG ("Searching for a net device for node " << GetNocNode ()->GetId () << " and direction " << routingDirection);
    Ptr<NocNetDevice> netDevice = 0;
    for (unsigned int i = 0; i < GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> tmpNetDevice = GetDevice (i)->GetObject<NocNetDevice> ();
        if (tmpNetDevice->GetRoutingDirection () == routingDirection)
          {
            netDevice = tmpNetDevice;
            break;
          }
      }
    if (netDevice)
      {
        NS_LOG_DEBUG ("Found net device " << netDevice->GetAddress ());
      }
    else
      {
        NS_LOG_DEBUG ("No net device found!");
      }
    return netDevice;
  }

  Ptr<NocNetDevice>
  FourWayRouter::GetOutputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection)
  {
    return GetInputNetDevice (sender, routingDirection);
  }

} // namespace ns3
