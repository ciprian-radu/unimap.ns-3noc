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

  NocRouter::NocRouter(std::string name)
  {
    m_name = name;
  }

  TypeId
  NocRouter::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::NocRouter")
        .SetParent<Object> ();
    return tid;
  }

  NocRouter::~NocRouter()
  {
    m_nocNode = 0;
    m_routingProtocol = 0;
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
  NocRouter::GetNDevices (void) const
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
