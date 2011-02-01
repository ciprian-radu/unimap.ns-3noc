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

#include "irvine-load-router.h"
#include "ns3/log.h"
#include "ns3/type-id.h"

NS_LOG_COMPONENT_DEFINE ("IrvineLoadRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (IrvineLoadRouter);

  TypeId
  IrvineLoadRouter::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::IrvineLoadRouter")
        .SetParent<IrvineRouter> ()
        .AddConstructor<IrvineLoadRouter> ()
        .AddAttribute ("LoadComponent",
            "the load router component",
            TypeIdValue (),
            MakeTypeIdAccessor (&IrvineLoadRouter::CreateLoadComponent),
            MakeTypeIdChecker ());
    return tid;
  }

  IrvineLoadRouter::IrvineLoadRouter () : IrvineRouter (__FILE__)
  {
    NS_LOG_LOGIC ("No load router component specified by constructor. "
        "Expecting that method SetLoadComponent(...) will be invoked later.");

    m_northLeftLoad = 0;
    m_northRightLoad = 0;
    m_eastLoad = 0;
    m_southLeftLoad = 0;
    m_southRightLoad = 0;
    m_westLoad = 0;
  }

  // we could easily name the router "Irvine load router", but using __FILE__ should be more useful for debugging
  IrvineLoadRouter::IrvineLoadRouter (Ptr<LoadRouterComponent> loadComponent) : IrvineRouter (__FILE__)
  {
    NS_ASSERT_MSG (loadComponent != 0, "The load router component must be specified!"
        " If you do not want to use a load router component, use another constructor.");
    m_loadComponent = loadComponent;
    NS_LOG_DEBUG ("Using the load router component " << loadComponent->GetName ());

    m_northLeftLoad = 0;
    m_northRightLoad = 0;
    m_eastLoad = 0;
    m_southLeftLoad = 0;
    m_southRightLoad = 0;
    m_westLoad = 0;
  }

  IrvineLoadRouter::~IrvineLoadRouter ()
  {
    ;
  }

  void
  IrvineLoadRouter::CreateLoadComponent (TypeId loadComponentTypeId)
  {
    ObjectFactory factory;
    factory.SetTypeId(loadComponentTypeId);
    m_loadComponent = factory.Create ()->GetObject<LoadRouterComponent> ();
    NS_LOG_DEBUG ("Using the load router component " << m_loadComponent->GetName ());
  }

  void
  IrvineLoadRouter::AddNeighborLoad (int load, Ptr<NocNetDevice> sourceDevice)
  {
    NS_ASSERT (sourceDevice != 0);

    Ptr<IrvineRouter> sourceRouter =
        sourceDevice->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetObject<IrvineRouter> ();
    NS_ASSERT (sourceRouter != 0);

    switch (sourceDevice->GetRoutingDirection ())
      {
        // TODO this router knows to work only with 2D meshes (it is only aware of NORTH, SOUTH, EAST, WEST directions)
        case NocRoutingProtocol::FORWARD:
          if (sourceDevice->GetRoutingDimension () == 1)
            {
              if (sourceRouter->isLeftRouter (sourceDevice))
                {
                  m_southLeftLoad = load;
                }
              else
                {
                  if (sourceRouter->isRightRouter (sourceDevice))
                    {
                      m_southRightLoad = load;
                    }
                  else
                    {
                      NS_LOG_ERROR ("The net device " << sourceDevice->GetAddress ()
                          << " does not belong to the left router, neither the right router!");
                    }
                }
            }
          else
            {
              if (sourceDevice->GetRoutingDimension () == 0)
                {
                  m_westLoad = load;
                }
            }
          break;
        case NocRoutingProtocol::BACK:
          if (sourceDevice->GetRoutingDimension () == 1)
            {
              if (sourceRouter->isLeftRouter (sourceDevice))
                {
                  m_northLeftLoad = load;
                }
              else
                {
                  if (sourceRouter->isRightRouter (sourceDevice))
                    {
                      m_northRightLoad = load;
                    }
                  else
                    {
                      NS_LOG_ERROR ("The net device " << sourceDevice->GetAddress ()
                          << " does not belong to the left router, neither the right router!");
                    }
                }
            }
          else
            {
              if (sourceDevice->GetRoutingDimension () == 0)
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
  IrvineLoadRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice)
  {
    NS_ASSERT (sourceDevice != 0);

    int load = 0;

    switch (sourceDevice->GetRoutingDirection ()) {
      case NocRoutingProtocol::FORWARD:
        if (sourceDevice->GetRoutingDimension () == 1)
          {
            if (isLeftRouter (sourceDevice))
              {
                load = m_southLeftLoad;
              }
            else
              {
                if (isRightRouter (sourceDevice))
                  {
                    load = m_southRightLoad;
                  }
                else
                  {
                    NS_LOG_ERROR ("The net device " << sourceDevice->GetAddress ()
                        << " does not belong to the left router, neither the right router!");
                  }
              }
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
            if (isLeftRouter (sourceDevice))
              {
                load = m_northLeftLoad;
              }
            else
              {
                if (isRightRouter (sourceDevice))
                  {
                    load = m_northRightLoad;
                  }
                else
                  {
                    NS_LOG_ERROR ("The net device " << sourceDevice->GetAddress ()
                        << " does not belong to the left router, neither the right router!");
                  }
              }
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
  IrvineLoadRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice, int direction, int dimension)
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
