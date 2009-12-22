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
        .SetParent<FourWayRouter> ();
    return tid;
  }

  // we could easily name the router "four way load router", but using __FILE__ should be more useful for debugging
  FourWayLoadRouter::FourWayLoadRouter (Ptr<LoadRouterComponent> loadComponent) : FourWayRouter (__FILE__)
  {
    NS_ASSERT_MSG (loadComponent != 0, "The load router component must be specified!"
        " If you do not want to use a load router component, use another constructor.");
    m_loadComponent = loadComponent;
    NS_LOG_DEBUG ("Using the load router component " << loadComponent->GetName ());
  }

  FourWayLoadRouter::~FourWayLoadRouter ()
  {
    ;
  }

} // namespace ns3
