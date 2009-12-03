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

#include "irvine-router.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/xy-routing.h"

NS_LOG_COMPONENT_DEFINE ("IrvineRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (IrvineRouter);

  TypeId
  IrvineRouter::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::IrvineRouter")
        .SetParent<NocRouter> ();
    return tid;
  }

  // we could easily name the router "Irvine router", but using __FILE__ should be more useful for debugging
  IrvineRouter::IrvineRouter() : NocRouter (__FILE__)
  {
    m_north1DeviceAdded = false;
    m_north2DeviceAdded = false;
    m_eastDeviceAdded = false;
    m_south1DeviceAdded = false;
    m_south2DeviceAdded = false;
    m_westDeviceAdded = false;
  }

  IrvineRouter::~IrvineRouter()
  {

  }

  bool
  IrvineRouter::RequestRoute(const Ptr<NocNetDevice> source, const Ptr<NocNode> destination,
      Ptr<Packet> packet, RouteReplyCallback routeReply)
  {
    NS_LOG_FUNCTION_NOARGS();
    GetRoutingProtocol()->RequestRoute (source, destination, packet, routeReply);
    return true;
  }

  /**
   * The device is added to the left or to the right router, based on its routing direction
   *
   * \see ns3::NocNetDevice#GetRoutingDirection()
   */
  uint32_t
  IrvineRouter::AddDevice (Ptr<NocNetDevice> device)
  {
    // we add all devices to m_devices as well
    uint32_t index = m_devices.size ();
    m_devices.push_back (device);
    NS_LOG_DEBUG ("Routing protocol is " << GetRoutingProtocol()->GetTypeId().GetName());
    // FIXME returns ns3::NocRoutingProtocol (not ns3::XyRouting)
//    if (GetRoutingProtocol()->GetTypeId().GetName().compare("ns3::XyRouting") == 0)
//      {
        switch (device->GetRoutingDirection ()) {
          case XyRouting::NONE:
            NS_LOG_WARN("The net device " << device->GetAddress () << " has no routing direction!");
            break;
          case XyRouting::NORTH:
            if (!m_north1DeviceAdded)
              {
                m_rightRouterDevices.push_back (device);
                m_north1DeviceAdded = true;
              }
            else
              {
                NS_ASSERT(!m_north2DeviceAdded);
                m_leftRouterDevices.push_back (device);
                m_north2DeviceAdded = true;
              }
            break;
          case XyRouting::EAST:
            NS_ASSERT(!m_eastDeviceAdded);
            m_rightRouterDevices.push_back (device);
            m_eastDeviceAdded = true;
            break;
          case XyRouting::SOUTH:
            if (!m_south1DeviceAdded)
              {
                m_rightRouterDevices.push_back (device);
                m_south1DeviceAdded = true;
              }
            else
              {
                NS_ASSERT(!m_south2DeviceAdded);
                m_leftRouterDevices.push_back (device);
                m_south2DeviceAdded = true;
              }
            break;
          case XyRouting::WEST:
            NS_ASSERT(!m_westDeviceAdded);
            m_leftRouterDevices.push_back (device);
            m_westDeviceAdded = true;
            break;
          default:
            break;
        }
//      }
//    else
//      {
//      NS_LOG_ERROR ("The Irvine router currently works only with XY routing!");
//      }
    return index;
  }

  Ptr<NocNetDevice>
  IrvineRouter::GetNetDevice(Ptr<NocNetDevice> sender, const int routingDirection)
  {
    NS_LOG_DEBUG ("Searching for a net device for node " << GetNocNode ()->GetId ()
        << " and direction " << routingDirection << " (sender net device is " << sender->GetAddress () << ")");

    NS_ASSERT(isRightRouter(sender) || isLeftRouter(sender));

    Ptr<NocNetDevice> netDevice = 0;
    if (isRightRouter(sender))
      {
        for (unsigned int i = 0; i < m_rightRouterDevices.size(); ++i)
          {
            Ptr<NocNetDevice> tmpNetDevice = m_rightRouterDevices[i]->GetObject<NocNetDevice> ();
            if (tmpNetDevice->GetRoutingDirection () == routingDirection)
              {
                netDevice = tmpNetDevice;
                break;
              }
          }
      }
    else
      {
      for (unsigned int i = 0; i < m_leftRouterDevices.size(); ++i)
        {
          Ptr<NocNetDevice> tmpNetDevice = m_leftRouterDevices[i]->GetObject<NocNetDevice> ();
          if (tmpNetDevice->GetRoutingDirection () == routingDirection)
            {
              netDevice = tmpNetDevice;
              break;
            }
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

  bool
  IrvineRouter::isRightRouter (Ptr<NocNetDevice> sender)
  {
    bool isRightRouter = false;
    Ptr<IrvineRouter> router = sender->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetObject<IrvineRouter> ();
    for (unsigned int i = 0; i < router->m_rightRouterDevices.size(); ++i)
      {
        NS_LOG_DEBUG ("Comparing " << router->m_rightRouterDevices[i]->GetAddress ()
            << " with " << sender->GetAddress ());
        if (router->m_rightRouterDevices[i] == sender)
          {
            isRightRouter = true;
            break;
          }
      }
    return isRightRouter;
  }

  bool
  IrvineRouter::isLeftRouter (Ptr<NocNetDevice> sender)
  {
    bool isLeftRouter = false;
    Ptr<IrvineRouter> router = sender->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetObject<IrvineRouter> ();
    for (unsigned int i = 0; i < router->m_leftRouterDevices.size(); ++i)
      {
      NS_LOG_DEBUG ("Comparing " << router->m_leftRouterDevices[i]->GetAddress ()
          << " with " << sender->GetAddress ());
        if (router->m_leftRouterDevices[i] == sender)
          {
            isLeftRouter = true;
            break;
          }
      }
    return isLeftRouter;
  }

} // namespace ns3
