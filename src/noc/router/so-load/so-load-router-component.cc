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
#include "so-load-router-component.h"

NS_LOG_COMPONENT_DEFINE ("SoLoadRouterComponent");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (SoLoadRouterComponent);

  SoLoadRouterComponent::SoLoadRouterComponent () : LoadRouterComponent (__FILE__)
  {
    ;
  }

  TypeId
  SoLoadRouterComponent::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::SoLoadRouterComponent")
        .SetParent<LoadRouterComponent> ()
        .AddConstructor<SoLoadRouterComponent> ();
    return tid;
  }

  SoLoadRouterComponent::~SoLoadRouterComponent ()
  {
    ;
  }

  int
  SoLoadRouterComponent::GetLocalLoad (Ptr<Packet> packet, Ptr<NocNetDevice> sourceDevice)
  {
    NS_ASSERT (sourceDevice != 0);
    int load = 0;

    Ptr<NocRouter> router = sourceDevice->GetNode ()->GetObject<NocNode> ()->GetRouter ();
    load += (int) router->GetInChannelsOccupancy (sourceDevice);
    load = load * 100;

    NS_ASSERT (load >= 0 && load <= 100);

    NS_LOG_DEBUG ("Retrieving local load: " << load);

    return load;
  }

  int
  SoLoadRouterComponent::GetLoadForDirection (Ptr<Packet> packet, Ptr<NocNetDevice> sourceDevice,
      Ptr<NocNetDevice> selectedDevice)
  {
    Ptr<NocRouter> router = sourceDevice->GetNode ()->GetObject<NocNode> ()->GetRouter ();
    int load = GetLocalLoad (packet, sourceDevice);
    m_load = 0;
    double neighbourLoad = 0;
    int counter = 0;

    NS_ASSERT (router);
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::FORWARD
        && selectedDevice->GetRoutingDimension () != 1)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::FORWARD, 1);
        counter++;
      }
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::FORWARD
        && selectedDevice->GetRoutingDimension () != 0)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::FORWARD, 0);
        counter++;
      }
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::BACK
        && selectedDevice->GetRoutingDimension () != 1)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::BACK, 1);
        counter++;
      }
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::BACK
        && selectedDevice->GetRoutingDimension () != 0)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::BACK, 0);
        counter++;
      }
    if (counter != 0)
      {
        neighbourLoad /= counter;
        load = (2 * load + (int)neighbourLoad) / 3;
      }

    NS_LOG_DEBUG ("The router given by net device " << sourceDevice->GetAddress ()
        << " provides the load " << load << " for direction " << selectedDevice->GetRoutingDirection ());

    return load;
  }

} // namespace ns3
