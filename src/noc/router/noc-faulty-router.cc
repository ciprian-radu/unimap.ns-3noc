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
#include "noc-faulty-router.h"
#include "ns3/noc-node.h"
#include "ns3/boolean.h"

NS_LOG_COMPONENT_DEFINE ("NocFaultyRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocFaultyRouter);

  NocFaultyRouter::NocFaultyRouter (std::string name) : NocRouter (name)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_faulty = false;
  }

  NocFaultyRouter::NocFaultyRouter (std::string name, Ptr<LoadRouterComponent> loadComponent) :
    NocRouter(name, loadComponent)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_faulty = false;
  }

  TypeId
  NocFaultyRouter::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::NocFaultyRouter")
        .SetParent<NocRouter> ()
        .AddAttribute (
            "Faulty",
            "Indicates if a router is faulty or not",
            BooleanValue (false),
            MakeBooleanAccessor (&NocFaultyRouter::m_faulty),
            MakeBooleanChecker ())
        ;
    return tid;
  }

  NocFaultyRouter::~NocFaultyRouter ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  Ptr<Route>
  NocFaultyRouter::ManageFlit (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> flit)
  {
    NS_LOG_FUNCTION_NOARGS();

    Ptr<Route> route = 0;
    if (!m_faulty)
      {
        route = NocRouter::ManageFlit (source, destination, flit);
      }
    else
      {
        NS_LOG_ERROR ("Cannot manage packet at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
    return route;
  }

  Ptr<LoadRouterComponent>
  NocFaultyRouter::GetLoadRouterComponent () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<LoadRouterComponent> loadComponent;
    if (!m_faulty)
      {
        loadComponent = m_loadComponent;
      }
    else
      {
        NS_LOG_ERROR ("Cannot retrieve the load router component at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
    return loadComponent;
  }

  void
  NocFaultyRouter::AddNeighborLoad (int load, Ptr<NocNetDevice> sourceDevice)
  {
    NS_LOG_FUNCTION_NOARGS ();

    // This method is meant to be overridden if you want it to do something
    if (m_faulty)
      {
        NS_LOG_ERROR ("Cannot add neighbor load at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  int
  NocFaultyRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice)
  {
    NS_LOG_FUNCTION_NOARGS ();

    // This method is meant to be overridden by the subclassing routers which work with load information
    if (m_faulty)
      {
        NS_LOG_ERROR ("Cannot get neighbor load at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
    return 0;
  }

  int
  NocFaultyRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice, int direction)
  {
    NS_LOG_FUNCTION_NOARGS ();

    // This method is meant to be overridden by the subclassing routers which work with load information
    if (m_faulty)
      {
        NS_LOG_ERROR ("Cannot get neighbor load at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
    return 0;
  }

  double
  NocFaultyRouter::GetInChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    NS_LOG_FUNCTION_NOARGS ();

    double occupancy = 0;

    if (!m_faulty)
      {
        occupancy = NocRouter::GetInChannelsOccupancy (sourceDevice);
      }
    else
      {
        NS_LOG_ERROR ("Cannot get in channels occupancy at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return occupancy;
  }

  double
  NocFaultyRouter::GetOutChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    NS_LOG_FUNCTION_NOARGS ();

    double occupancy = 0;

    if (!m_faulty)
      {
        occupancy = NocRouter::GetOutChannelsOccupancy (sourceDevice);
      }
    else
      {
        NS_LOG_ERROR ("Cannot get out channels occupancy at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return occupancy;
  }

  void
  NocFaultyRouter::SetRoutingProtocol (Ptr<NocRoutingProtocol> routingProtocol)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocRouter::SetRoutingProtocol (routingProtocol);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set the routing protocol at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  Ptr<NocRoutingProtocol>
  NocFaultyRouter::GetRoutingProtocol ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NocRoutingProtocol> routingProtocol = 0;

    if (!m_faulty)
      {
        routingProtocol = NocRouter::GetRoutingProtocol ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get the routing protocol at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return routingProtocol;
  }

  void
  NocFaultyRouter::SetSwitchingProtocol (Ptr<NocSwitchingProtocol> switchingProtocol)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocRouter::SetSwitchingProtocol (switchingProtocol);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set the switching protocol at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  Ptr<NocSwitchingProtocol>
  NocFaultyRouter::GetSwitchingProtocol ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NocSwitchingProtocol> switchingProtocol = 0;

    if (!m_faulty)
      {
        switchingProtocol = NocRouter::GetSwitchingProtocol ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get the switching protocol at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return switchingProtocol;
  }

  uint32_t
  NocFaultyRouter::AddDevice (Ptr<NocNetDevice> device)
  {
    NS_LOG_FUNCTION_NOARGS ();

    uint32_t index = 0;

    if (!m_faulty)
      {
        index = NocRouter::AddDevice (device);
      }
    else
      {
        NS_LOG_ERROR ("Cannot add device at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return index;
  }

  Ptr<NocNetDevice>
  NocFaultyRouter::GetDevice (uint32_t index) const
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NocNetDevice> device = 0;

    if (!m_faulty)
      {
        device = NocRouter::GetDevice (index);
      }
    else
      {
        NS_LOG_ERROR ("Cannot get device at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return device;
  }

  uint32_t
  NocFaultyRouter::GetNDevices () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    uint32_t n;

    if (!m_faulty)
      {
        n = NocRouter::GetNDevices();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get the number of devices at node " << (int) m_nocNode->GetId ()
            << " because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return n;
  }

  void
  NocFaultyRouter::SetNocNode (Ptr<NocNode> nocNode)
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (!m_faulty)
      {
        NocRouter::SetNocNode (nocNode);
      }
    else
      {
        NS_LOG_ERROR ("Cannot set the node because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }
  }

  Ptr<NocNode>
  NocFaultyRouter::GetNocNode () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NocNode> nocNode;

    if (!m_faulty)
      {
        nocNode = NocRouter::GetNocNode ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get the node because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return nocNode;
  }

  std::string
  NocFaultyRouter::GetName () const
  {
    NS_LOG_FUNCTION_NOARGS ();

    std::string name;

    if (!m_faulty)
      {
        name = NocRouter::GetName ();
      }
    else
      {
        NS_LOG_ERROR ("Cannot get the node because the router is faulty!");
        // just force the simulation to stop
        NS_ASSERT (!m_faulty);
      }

    return name;
  }

} // namespace ns3
