/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
#include "noc-application-helper.h"
#include "ns3/noc-application.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/net-device-container.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3
{

  NocApplicationHelper::NocApplicationHelper (NodeContainer nodes, NetDeviceContainer devices, uint32_t hSize)
  {
    m_nodes = nodes;
    m_devices = devices;
    m_factory.SetTypeId ("ns3::NocApplication");
    m_factory.Set ("HSize", UintegerValue (hSize));
  }

  void
  NocApplicationHelper::SetAttribute (std::string name, const AttributeValue &value)
  {
    m_factory.Set (name, value);
  }

  ApplicationContainer
  NocApplicationHelper::Install (Ptr<Node> node) const
  {
    return ApplicationContainer (InstallPriv (node));
  }

  ApplicationContainer
  NocApplicationHelper::Install (std::string nodeName) const
  {
    Ptr<Node> node = Names::Find<Node> (nodeName);
    return ApplicationContainer (InstallPriv (node));
  }

  ApplicationContainer
  NocApplicationHelper::Install (NodeContainer c) const
  {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
      {
        apps.Add (InstallPriv (*i));
      }

    return apps;
  }

  Ptr<Application>
  NocApplicationHelper::InstallPriv (Ptr<Node> node) const
  {
    Ptr<NocApplication> app = m_factory.Create<NocApplication> ();
    app->SetNetDeviceContainer (m_devices);
    app->SetNodeContainer (m_nodes);
    node->AddApplication (app);

    return app;
  }

} // namespace ns3
