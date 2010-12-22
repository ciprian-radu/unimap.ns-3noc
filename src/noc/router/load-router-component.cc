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
#include "load-router-component.h"

NS_LOG_COMPONENT_DEFINE ("LoadRouterComponent");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (LoadRouterComponent);

  LoadRouterComponent::LoadRouterComponent (std::string name)
  {
    m_name = name;
    m_load = 0;
  }

  TypeId
  LoadRouterComponent::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::LoadRouterComponent")
        .SetParent<Object> ();
    return tid;
  }

  LoadRouterComponent::~LoadRouterComponent ()
  {
    ;
  }

  void
  LoadRouterComponent::IncreaseLoad ()
  {
    m_load++;
    NS_LOG_DEBUG ("Load increased to " << m_load);
  }

  std::string
  LoadRouterComponent::GetName ()
  {
    return m_name;
  }

} // namespace ns3
