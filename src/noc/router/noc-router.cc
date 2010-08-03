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
#include "noc-router.h"
#include "ns3/noc-node.h"

NS_LOG_COMPONENT_DEFINE ("NocRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocRouter);

  NocRouter::NocRouter (std::string name)
  {
    m_name = name;
    m_loadComponent = 0;
    NS_LOG_DEBUG ("No load router component is used");
  }

  NocRouter::NocRouter (std::string name, Ptr<LoadRouterComponent> loadComponent)
  {
    NS_ASSERT_MSG (loadComponent != 0, "The load router component must be specified!"
        " If you do not want to use a load router component, use another constructor.");
    m_name = name;
    m_loadComponent = loadComponent;
    NS_LOG_DEBUG ("Using the load router component " << loadComponent->GetName ());
  }

  TypeId
  NocRouter::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::NocRouter")
        .SetParent<Object> ();
    return tid;
  }

  NocRouter::~NocRouter ()
  {
    m_nocNode = 0;
    m_routingProtocol = 0;
  }

  Ptr<Route>
  NocRouter::ManagePacket (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT (source != 0);
    NS_ASSERT (destination != 0);

    uint32_t sourceNodeId = source->GetNode ()->GetId ();
    uint32_t destinationNodeId = destination->GetId ();
    Ptr<Route> route = 0;
    if (sourceNodeId == destinationNodeId)
      {
        NS_LOG_WARN ("Trying to route a packet from node " << sourceNodeId
            << " to node " << destinationNodeId << " (same node)");
      }
    else
      {
        route = GetRoutingProtocol()->RequestRoute (source, destination, packet);
      }
    return route;
  }

  Ptr<LoadRouterComponent>
  NocRouter::GetLoadRouterComponent () const
  {
    return m_loadComponent;
  }

  void
  NocRouter::AddNeighborLoad (int load, Ptr<NocNetDevice> sourceDevice)
  {
    ; // This method is meant to be overridden if you want it to do something
  }

  int
  NocRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice)
  {
    // This method is meant to be overridden by the subclassing routers which work with load information
    return 0;
  }

  int
  NocRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice, int direction)
  {
    // This method is meant to be overridden by the subclassing routers which work with load information
    return 0;
  }

  double
  NocRouter::GetInChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    double occupancy = 0;
    uint32_t packets = 0;
    uint64_t sizes = 0;

    for (uint32_t i = 0; i < GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> device = GetDevice (i);
        packets += device->GetInQueueNPacktes ();
        sizes += device->GetInQueueSize ();
      }
    if (sizes != 0 && packets != 0)
      {
        occupancy = packets * 1.0 / sizes;
      }
    NS_LOG_LOGIC ("In channels occupancy is " << occupancy);

    return occupancy;
  }

  double
  NocRouter::GetOutChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    double occupancy = 0;
    uint32_t packets = 0;
    uint64_t sizes = 0;

    for (uint32_t i = 0; i < GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> device = GetDevice (i);
        packets += device->GetOutQueueNPacktes ();
        sizes += device->GetOutQueueSize ();
      }
    if (sizes != 0 && packets != 0)
      {
        occupancy = packets * 1.0 / sizes;
      }
    NS_LOG_LOGIC ("Out channels occupancy is " << occupancy);

    return occupancy;
  }

  void
  NocRouter::SetRoutingProtocol (Ptr<NocRoutingProtocol> routingProtocol)
  {
    std::ostringstream oss;
    oss << "Setting a '" << routingProtocol->GetName ()
        << "' routing protocol for the router of the NoC node " << m_nocNode->GetId ();
    NS_LOG_DEBUG (oss.str());

    m_routingProtocol = routingProtocol;
  }

  Ptr<NocRoutingProtocol>
  NocRouter::GetRoutingProtocol ()
  {
    if (m_routingProtocol == 0)
      {
        std::ostringstream oss;
        oss << "No routing protocol is defined for the NoC node " << m_nocNode-> GetId ();
        NS_LOG_WARN(oss.str());
      }
    return m_routingProtocol;
  }

  void
  NocRouter::SetSwitchingProtocol (Ptr<NocSwitchingProtocol> switchingProtocol)
  {
    std::ostringstream oss;
    oss << "Setting a '" << switchingProtocol->GetName ()
        << "' switching protocol for the router of the NoC node " << m_nocNode->GetId ();
    NS_LOG_DEBUG (oss.str());

    m_switchingProtocol = switchingProtocol;
  }

  Ptr<NocSwitchingProtocol>
  NocRouter::GetSwitchingProtocol ()
  {
    if (m_switchingProtocol == 0)
      {
        std::ostringstream oss;
        oss << "No switching protocol is defined for the NoC node " << m_nocNode-> GetId ();
        NS_LOG_WARN(oss.str());
      }
    return m_switchingProtocol;
  }

  uint32_t
  NocRouter::AddDevice (Ptr<NocNetDevice> device)
  {
    uint32_t index = m_devices.size ();
    m_devices.push_back (device);
    return index;
  }

  Ptr<NocNetDevice>
  NocRouter::GetDevice (uint32_t index) const
  {
    NS_ASSERT_MSG (index < m_devices.size (), "Device index " << index <<
                   " is out of range (only have " << m_devices.size () << " devices).");
    return m_devices[index];
  }

  uint32_t
  NocRouter::GetNDevices () const
  {
    return m_devices.size ();
  }

  void
  NocRouter::SetNocNode(Ptr<NocNode> nocNode)
  {
    m_nocNode = nocNode;
  }

  Ptr<NocNode>
  NocRouter::GetNocNode() const
  {
    return m_nocNode;
  }

  std::string
  NocRouter::GetName() const
  {
   return m_name;
  }

} // namespace ns3
