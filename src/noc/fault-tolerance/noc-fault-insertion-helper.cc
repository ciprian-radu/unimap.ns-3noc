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

#include "noc-fault-insertion-helper.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/noc-node.h"
#include "ns3/noc-router.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"

NS_LOG_COMPONENT_DEFINE ("NocFaultInsertionHelper");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocFaultInsertionHelper);

  const std::string NocFaultInsertionHelper::FAULTY_ATTRIBUTE = "Faulty";

  TypeId
  NocFaultInsertionHelper::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::NocFaultInsertionHelper")
        .SetParent<Object> ()
        ;
    return tid;
  }

  NocFaultInsertionHelper::NocFaultInsertionHelper (NodeContainer nodeContainer)
  {
    m_nodeContainer = nodeContainer;
  }

  NocFaultInsertionHelper::~NocFaultInsertionHelper ()
  {
    ;
  }

  void
  NocFaultInsertionHelper::SetNodeAsFaulty (uint32_t nodeId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Node> node = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        if (node->GetId () == nodeId)
          {
            break;
          }
      }
    if (node == 0)
      {
        NS_LOG_WARN ("Cannot set node " << (int) nodeId
            << " as faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
//        bool attributeFound = node->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        node->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (faulty.Get ())
          {
            NS_LOG_WARN ("The node " << (int) nodeId << " is already faulty!");
          }
        else
          {
            faulty.Set (true);
            node->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetNodeAsFaultyAfterDelay (uint32_t nodeId, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetNodeAsFaulty, this, nodeId);
  }

  void
  NocFaultInsertionHelper::SetNodeAsNotFaulty (uint32_t nodeId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Node> node = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        if (node->GetId () == nodeId)
          {
            break;
          }
      }
    if (node == 0)
      {
        NS_LOG_WARN ("Cannot set node " << (int) nodeId
            << " as not faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
//        bool attributeFound = node->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        node->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (!faulty.Get ())
          {
            NS_LOG_WARN ("The node " << (int) nodeId << " is already not faulty!");
          }
        else
          {
            faulty.Set (false);
            node->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetNodeAsNotFaultyAfterDelay (uint32_t nodeId, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetNodeAsNotFaulty, this, nodeId);
  }

  void
  NocFaultInsertionHelper::SetNodeRouterAsFaulty (uint32_t nodeId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Node> node = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        if (node->GetId () == nodeId)
          {
            break;
          }
      }
    if (node == 0)
      {
        NS_LOG_WARN ("Cannot set the router of node " << (int) nodeId
            << " as faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
        Ptr<NocRouter> router = node->GetObject<NocNode> ()->GetRouter ();
        NS_ASSERT (router != 0);
//        bool attributeFound = router->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        router->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (faulty.Get ())
          {
            NS_LOG_WARN ("The router of the node " << (int) nodeId << " is already faulty!");
          }
        else
          {
            faulty.Set (true);
            router->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetNodeRouterAsFaultyAfterDelay (uint32_t nodeId, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetNodeRouterAsFaulty, this, nodeId);
  }

  void
  NocFaultInsertionHelper::SetNodeRouterAsNotFaulty (uint32_t nodeId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Node> node = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        if (node->GetId () == nodeId)
          {
            break;
          }
      }
    if (node == 0)
      {
        NS_LOG_WARN ("Cannot set the router of node " << (int) nodeId
            << " as not faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
        Ptr<NocRouter> router = node->GetObject<NocNode> ()->GetRouter ();
        NS_ASSERT (router != 0);
//        bool attributeFound = router->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        router->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (!faulty.Get ())
          {
            NS_LOG_WARN ("The router of the node " << (int) nodeId << " is already not faulty!");
          }
        else
          {
            faulty.Set (false);
            router->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetNodeRouterAsNotFaultyAfterDelay (uint32_t nodeId, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetNodeRouterAsNotFaulty, this, nodeId);
  }

  void
  NocFaultInsertionHelper::SetNetDeviceAsFaulty (Address netDeviceAddress)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NetDevice> device = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        Ptr<Node> node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        for (uint32_t j = 0; j < node->GetNDevices (); ++j)
          {
            Ptr<NetDevice> dev = node->GetDevice (j);
            NS_ASSERT (dev != 0);
            if (dev->GetAddress () == netDeviceAddress)
              {
                device = dev;
                goto L;
              }
          }
      }
    L :
    if (device == 0)
      {
        NS_LOG_WARN ("Cannot set net device " << netDeviceAddress
            << " as faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
//        bool attributeFound = device->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        device->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (faulty.Get ())
          {
            NS_LOG_WARN ("The net device " << netDeviceAddress << " is already faulty!");
          }
        else
          {
            faulty.Set (true);
            device->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetNetDeviceAsFaultyAfterDelay (Address netDeviceAddress, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetNetDeviceAsFaulty, this, netDeviceAddress);
  }

  void
  NocFaultInsertionHelper::SetNetDeviceAsNotFaulty (Address netDeviceAddress)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NetDevice> device = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        Ptr<Node> node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        for (uint32_t j = 0; j < node->GetNDevices (); ++j)
          {
            Ptr<NetDevice> dev = node->GetDevice (j);
            NS_ASSERT (dev != 0);
            if (dev->GetAddress () == netDeviceAddress)
              {
                device = dev;
                goto L;
              }
          }
      }
    L :
    if (device == 0)
      {
        NS_LOG_WARN ("Cannot set net device " << netDeviceAddress
            << " as not faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
//        bool attributeFound = device->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        device->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (!faulty.Get ())
          {
            NS_LOG_WARN ("The net device " << netDeviceAddress << " is already not faulty!");
          }
        else
          {
            faulty.Set (false);
            device->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetNetDeviceAsNotFaultyAfterDelay (Address netDeviceAddress, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetNetDeviceAsNotFaulty, this, netDeviceAddress);
  }

  void
  NocFaultInsertionHelper::SetChannelAsFaulty (uint32_t channelId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Channel> channel = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        Ptr<Node> node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        for (uint32_t j = 0; j < node->GetNDevices (); ++j)
          {
            Ptr<NetDevice> dev = node->GetDevice (j);
            NS_ASSERT (dev != 0);
            Ptr<Channel> ch = dev->GetChannel ();
            if (ch != 0)
              {
                if (ch->GetId () == channelId)
                  {
                    channel = ch;
                    goto L;
                  }
              }
          }
      }
    L :
    if (channel == 0)
      {
        NS_LOG_WARN ("Cannot set channel " << channelId
            << " as faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
//        bool attributeFound = device->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        channel->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (faulty.Get ())
          {
            NS_LOG_WARN ("The channel " << channelId << " is already faulty!");
          }
        else
          {
            faulty.Set (true);
            channel->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetChannelAsFaultyAfterDelay (uint32_t channelId, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetChannelAsFaulty, this, channelId);
  }

  void
  NocFaultInsertionHelper::SetChannelAsNotFaulty (uint32_t channelId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<Channel> channel = 0;
    for (uint32_t i = 0; i < m_nodeContainer.GetN (); ++i)
      {
        Ptr<Node> node = m_nodeContainer.Get (i);
        NS_ASSERT (node != 0);
        for (uint32_t j = 0; j < node->GetNDevices (); ++j)
          {
            Ptr<NetDevice> dev = node->GetDevice (j);
            NS_ASSERT (dev != 0);
            Ptr<Channel> ch = dev->GetChannel ();
            if (ch != 0)
              {
                if (ch->GetId () == channelId)
                  {
                    channel = ch;
                    goto L;
                  }
              }
          }
      }
    L :
    if (channel == 0)
      {
        NS_LOG_WARN ("Cannot set channel " << channelId
            << " as not faulty because it was not found in the node container");
      }
    else
      {
        BooleanValue faulty;
//        bool attributeFound = device->GetAttributeFailSafe (FAULTY_ATTRIBUTE, faulty);
        channel->GetAttribute (FAULTY_ATTRIBUTE, faulty);
        if (!faulty.Get ())
          {
            NS_LOG_WARN ("The channel " << channelId << " is already not faulty!");
          }
        else
          {
            faulty.Set (false);
            channel->SetAttribute (FAULTY_ATTRIBUTE, faulty);
          }
      }
  }

  void
  NocFaultInsertionHelper::SetChannelAsNotFaultyAfterDelay (uint32_t channelId, Time delay)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Schedule(delay, &NocFaultInsertionHelper::SetChannelAsNotFaulty, this, channelId);
  }

} // namespace ns3
