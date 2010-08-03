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
  SoLoadRouterComponent::GetLocalLoad ()
  {
    int load = 0;
    
//    int count = 1;
//        int load = router.getInjectionChannel().getFlitsInBufferCount();
//        if(load > StatisticsController.getInstance().getBufferSize()) {
//                load = StatisticsController.getInstance().getBufferSize();
//        }
//        if(router.getNorthInChannel() != null) {
//                count++;
//                load += router.getNorthInChannel().getFlitsInBufferCount();
//        }
//        if(router.getSouthInChannel() != null) {
//                count++;
//                load += router.getSouthInChannel().getFlitsInBufferCount();
//        }
//        if(router.getEastInChannel() != null) {
//                count++;
//                load += router.getEastInChannel().getFlitsInBufferCount();
//        }
//        if(router.getWestInChannel() != null) {
//                count++;
//                load += router.getWestInChannel().getFlitsInBufferCount();
//        }
//        load = (int)(load / (float)(count * StatisticsController.getInstance().getBufferSize()) * 100);

    // FIXME

    NS_ASSERT (load >= 0 && load <= 100);

    NS_LOG_DEBUG ("Retrieving local load: " << load);

    return load;
  }

  int
  SoLoadRouterComponent::GetLoadForDirection (Ptr<NocNetDevice> sourceDevice, Ptr<NocNetDevice> selectedDevice)
  {
    int load = GetLocalLoad ();
    double neighbourLoad = 0;
    int counter = 0;

    Ptr<NocRouter> router = sourceDevice->GetNode ()->GetObject<NocNode> ()->GetRouter ();
    NS_ASSERT (router);
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::NORTH)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::NORTH);
        counter++;
      }
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::EAST)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::EAST);
        counter++;
      }
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::SOUTH)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::SOUTH);
        counter++;
      }
    if (selectedDevice->GetRoutingDirection () != NocRoutingProtocol::WEST)
      {
        neighbourLoad += router->GetNeighborLoad (sourceDevice, NocRoutingProtocol::WEST);
        counter++;
      }
    if (counter != 0)
      {
        neighbourLoad /= counter;
        load = (2 * load + neighbourLoad) / 3;
      }

    NS_LOG_DEBUG ("The router given by net device " << sourceDevice->GetAddress ()
        << " provides the load " << load << " for direction " << selectedDevice->GetRoutingDirection ());

    return load;
  }

} // namespace ns3
