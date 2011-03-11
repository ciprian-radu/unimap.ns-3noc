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
#include "slb-load-router-component.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/noc-application.h"
#include "ns3/integer.h"
#include "ns3/noc-registry.h"

NS_LOG_COMPONENT_DEFINE ("SlbLoadRouterComponent");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (SlbLoadRouterComponent);

  SlbLoadRouterComponent::SlbLoadRouterComponent() : LoadRouterComponent (__FILE__)
  {
    m_dataLength = -1;
  }

  TypeId
  SlbLoadRouterComponent::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::SlbLoadRouterComponent")
        .SetParent<LoadRouterComponent> ()
        .AddConstructor<SlbLoadRouterComponent> ();
    return tid;
  }

  SlbLoadRouterComponent::~SlbLoadRouterComponent ()
  {
    ;
  }

  int
  SlbLoadRouterComponent::GetLocalLoad (Ptr<Packet> packet, Ptr<NocNetDevice> sourceDevice)
  {
    // Note that we don't really account for router specific data
    // (this is a static algorithm)
    int load = 0;
    // load = (int)((router.getNewLoad() / (8.0f * (6.0f * router.getDataFlitSpeedup() + router.getNode().getProcessingElement().getMessageLength()))) * 100.0f);
    IntegerValue speedup;
    NocRegistry::GetInstance ()->GetAttribute ("DataPacketSpeedup", speedup);
    int dataFlitSpeedup = speedup.Get ();

    NS_ASSERT (sourceDevice != 0);
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        NocHeader header;
        packet->PeekHeader (header);
        if (!header.IsEmpty ())
          {
            m_dataLength = tag.GetDataFlitCount ();
          }
      }
    NS_LOG_DEBUG ("Message has " << m_dataLength << " data packets");
    NS_ASSERT (m_dataLength >= 0);

    NS_LOG_DEBUG ("Current load is " << m_load);

    load = (int) ((m_load / (8.0 * (6.0 * dataFlitSpeedup + m_dataLength))) * 100);

    // The above formula does not guarantee that load <= 100
    // (but that is the formula used by the algorithm...)
    if (load > 100)
      {
        NS_LOG_WARN ("The load is " << load << " It is set to 100 (the maximum allowed value)");
        load = 100;
      }

    NS_LOG_DEBUG ("Retrieving local load: " << load);
    NS_ASSERT (load >= 0 && load <= 100);

    return load;
  }

  int
  SlbLoadRouterComponent::GetLoadForDirection (Ptr<Packet> packet, Ptr<NocNetDevice> sourceDevice,
      Ptr<NocNetDevice> selectedDevice)
  {
    int load = GetLocalLoad (packet, sourceDevice);
    m_load = 0;
    double neighbourLoad = 0;
    int counter = 0;

    Ptr<NocRouter> router = sourceDevice->GetNode ()->GetObject<NocNode> ()->GetRouter ();
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
