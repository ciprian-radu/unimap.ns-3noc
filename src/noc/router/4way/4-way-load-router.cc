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

#include "4-way-load-router.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"

NS_LOG_COMPONENT_DEFINE ("FourWayLoadRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (FourWayLoadRouter);

  TypeId
  FourWayLoadRouter::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::FourWayLoadRouter")
        .SetParent<FourWayRouter> ()
        .AddConstructor<FourWayLoadRouter> ()
        .AddAttribute ("LoadComponent",
            "the load router component",
            TypeIdValue (),
            MakeTypeIdAccessor (&FourWayLoadRouter::CreateLoadComponent),
            MakeTypeIdChecker ());
    return tid;
  }

  FourWayLoadRouter::FourWayLoadRouter () : FourWayRouter (__FILE__)
  {
    NS_LOG_LOGIC ("No load router component specified by constructor. "
        "Expecting that method SetLoadComponent(...) will be invoked later.");

    m_northLoad = 0;
    m_eastLoad = 0;
    m_southLoad = 0;
    m_westLoad = 0;
  }

  // we could easily name the router "four way load router", but using __FILE__ should be more useful for debugging
  FourWayLoadRouter::FourWayLoadRouter (Ptr<LoadRouterComponent> loadComponent) : FourWayRouter (__FILE__)
  {
    NS_ASSERT_MSG (loadComponent != 0, "The load router component must be specified!"
        " If you do not want to use a load router component, use another constructor.");
    m_loadComponent = loadComponent;
    NS_LOG_DEBUG ("Using the load router component " << loadComponent->GetName ());

    m_northLoad = 0;
    m_eastLoad = 0;
    m_southLoad = 0;
    m_westLoad = 0;
  }

  FourWayLoadRouter::~FourWayLoadRouter ()
  {
    ;
  }

  void
  FourWayLoadRouter::CreateLoadComponent (TypeId loadComponentTypeId)
  {
    ObjectFactory factory;
    factory.SetTypeId(loadComponentTypeId);
    m_loadComponent = factory.Create ()->GetObject<LoadRouterComponent> ();
    NS_LOG_DEBUG ("Using the load router component " << m_loadComponent->GetName ());
  }

  void
  FourWayLoadRouter::AddNeighborLoad (int load, Ptr<NocNetDevice> sourceDevice)
  {
    NS_ASSERT (sourceDevice != 0);

    switch (sourceDevice->GetRoutingDirection ())
      {
        // TODO this router knows to work only with 2D meshes (it is only aware of NORTH, SOUTH, EAST, WEST directions)
        case NocRoutingProtocol::FORWARD:
          if (sourceDevice->GetRoutingDimension () == 1)
            {
              m_southLoad = load;
            }
          else
            {
              if (sourceDevice->GetRoutingDirection () == 0)
                {
                  m_westLoad = load;
                }
            }
          break;
        case NocRoutingProtocol::BACK:
          if (sourceDevice->GetRoutingDimension () == 1)
            {
              m_northLoad = load;
            }
          else
            {
              if (sourceDevice->GetRoutingDirection () == 0)
                {
                  m_eastLoad = load;
                }
            }
          break;
        case NocRoutingProtocol::NONE:
        default:
          NS_LOG_ERROR ("Unknown routing direction!");
          break;
      }
  }

  int
  FourWayLoadRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice)
  {
    NS_ASSERT (sourceDevice != 0);

    int load = 0;

    switch (sourceDevice->GetRoutingDirection ()) {
      case NocRoutingProtocol::FORWARD:
        if (sourceDevice->GetRoutingDimension () == 1)
          {
            load = m_southLoad;
          }
        else
          {
            if (sourceDevice->GetRoutingDimension () == 0)
              {
                load = m_westLoad;
              }
          }
        break;
      case NocRoutingProtocol::BACK:
        if (sourceDevice->GetRoutingDimension () == 1)
          {
            load = m_northLoad;
          }
        else
          {
            if (sourceDevice->GetRoutingDimension () == 0)
              {
                load = m_eastLoad;
              }
          }
        break;
      case NocRoutingProtocol::NONE:
      default:
        NS_LOG_ERROR ("Unknown routing direction!");
        break;
    }

    return load;
  }

  int
  FourWayLoadRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice, int direction, int dimension)
  {
    NS_ASSERT (sourceDevice != 0);
    int load = 0;

    NS_LOG_DEBUG ("Requesting neighbor load (source net device is "
        << sourceDevice->GetAddress () << ", direction is " << direction);

    Ptr<NocNetDevice> device = GetInputNetDevice (sourceDevice, direction, dimension);
    if (device == 0)
      {
        NS_LOG_WARN ("No input net device was found based on source device "
            << sourceDevice->GetAddress () << " and direction " << direction);
        load = 0;
      }
    else
      {
        load = GetNeighborLoad (device);
      }
    NS_LOG_DEBUG ("Retrieving neighbor load " << load);

    return load;
  }


} // namespace ns3
