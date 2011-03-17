/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *               						Lucian Blaga University of Sibiu, Romania
 *               - Systems and Networking, University of Augsburg, Germany
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
 * Author: Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *         http://webspace.ulbsibiu.ro/ciprian.radu/
 */

#include "noc-irvine-mesh-2d.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/xy-routing.h"
#include "ns3/4-way-router.h"
#include "ns3/irvine-router.h"
#include "ns3/saf-switching.h"
#include "ns3/wormhole-switching.h"
#include "ns3/vct-switching.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/integer.h"
#include "ns3/file-utils.h"

NS_LOG_COMPONENT_DEFINE ("NocIrvineMesh2D");;

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocIrvineMesh2D);

  TypeId
  NocIrvineMesh2D::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::NocIrvineMesh2D")
        .SetParent<NocTopology> ()
        .AddConstructor<NocIrvineMesh2D> ()
        .AddAttribute ("hSize",
            "how many nodes the 2D mesh will have on one horizontal line",
            IntegerValue (4),
            MakeIntegerAccessor (&NocIrvineMesh2D::m_hSize),
            MakeIntegerChecker<int> (1, 127));
    return tid;
  }

  NocIrvineMesh2D::NocIrvineMesh2D () : NocTopology ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  NocIrvineMesh2D::~NocIrvineMesh2D ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  NetDeviceContainer
  NocIrvineMesh2D::Install (NodeContainer nodes)
  {
    NS_LOG_FUNCTION_NOARGS ();
    NS_LOG_DEBUG ("hSize " << m_hSize);

    m_nodes = nodes;

    Ptr<NocChannel> channel = 0;
    Ptr<NocNetDevice> netDevice;

    for (unsigned int i = 0; i < nodes.GetN (); ++i)
      {
        Ptr<NocNode> nocNode = nodes.Get (i)->GetObject<NocNode> ();
        if (nocNode->GetRouter () == 0)
          {
            Ptr<NocRouter> router = m_routerFactory.Create ()->GetObject<NocRouter> ();
            router->SetNocNode (nocNode);
            nocNode->SetRouter (router);
            //            Callback<ObjectBase *> constructorRouting = routingProtocolTid.GetConstructor ();
            //            NocRoutingProtocol *ptrRouting = dynamic_cast<NocRoutingProtocol *> (constructorRouting ());
            //            Ptr<NocRoutingProtocol> routingProtocol = Ptr<NocRoutingProtocol> (ptrRouting);
            Ptr<NocRoutingProtocol> m_routingProtocol = m_routingProtocolFactory.Create ()->GetObject<NocRoutingProtocol> ();
            router->SetRoutingProtocol (m_routingProtocol);

            //            Callback<ObjectBase *> constructorSwitching = switchingProtocolTid.GetConstructor ();
            //            NocSwitchingProtocol *ptrSwitching = dynamic_cast<NocSwitchingProtocol *> (constructorSwitching ());
            //            Ptr<NocSwitchingProtocol> switchingProtocol = Ptr<NocSwitchingProtocol> (ptrSwitching);
            Ptr<NocSwitchingProtocol> switchingProtocol =
                m_switchingProtocolFactory.Create ()->GetObject<NocSwitchingProtocol> ();
            router->SetSwitchingProtocol (switchingProtocol);
          }
      }

    // create the horizontal channels (and net devices)
    for (unsigned int i = 0; i < nodes.GetN (); ++i)
      {
        Ptr<NocNode> nocNode = nodes.Get (i)->GetObject<NocNode> ();
        if (channel != 0)
          {
            netDevice = CreateObject<NocNetDevice> ();
            netDevice->SetAddress (Mac48Address::Allocate ());
            netDevice->SetChannel (channel);
            netDevice->SetRoutingDirection (NocRoutingProtocol::BACK, 0); // WEST
            m_devices.Add (netDevice);
            // attach input buffering (we don't use output buffering for the moment)
            Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
            netDevice->SetInQueue (inQueue);
            nocNode->AddDevice (netDevice);
            nocNode->GetRouter ()->AddDevice (netDevice);
          }

        if (i == 0 || (i > 0 && (i + 1) % m_hSize != 0))
          {
            channel = m_channelFactory.Create ()->GetObject<NocChannel> ();
            netDevice = CreateObject<NocNetDevice> ();
            netDevice->SetAddress (Mac48Address::Allocate ());
            netDevice->SetChannel (channel);
            netDevice->SetRoutingDirection (NocRoutingProtocol::FORWARD, 0); // EAST
            m_devices.Add (netDevice);
            // attach input buffering (we don't use output buffering for the moment)
            Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
            netDevice->SetInQueue (inQueue);
            nocNode->AddDevice (netDevice);
            nocNode->GetRouter ()->AddDevice (netDevice);
          }
        else
          {
            channel = 0;
          }
      }

    // create the vertical channels (and net devices)
    channel = 0;
    // The Irvine architecture has 2 channels for both North and South ports of each node
    std::vector<Ptr<NocChannel> > columnChannels (2 * m_hSize);
    for (unsigned int i = 0; i < nodes.GetN (); i = i + m_hSize)
      {
        for (int j = 0; j < 2 * m_hSize; ++j)
          {
            Ptr<NocNode> nocNode = nodes.Get (i + j % m_hSize)->GetObject<NocNode> ();
            if (columnChannels[j] != 0)
              {
                channel = columnChannels[j];
                netDevice = CreateObject<NocNetDevice> ();
                netDevice->SetAddress (Mac48Address::Allocate ());
                netDevice->SetChannel (channel);
                netDevice->SetRoutingDirection (NocRoutingProtocol::FORWARD, 1); // NORTH
                m_devices.Add (netDevice);
                // attach input buffering (we don't use output buffering for the moment)
                Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                netDevice->SetInQueue (inQueue);
                nocNode->AddDevice (netDevice);
                nocNode->GetRouter ()->AddDevice (netDevice);
              }
            if (i < nodes.GetN () - m_hSize)
              {
                channel = m_channelFactory.Create ()->GetObject<NocChannel> ();
                netDevice = CreateObject<NocNetDevice> ();
                netDevice->SetAddress (Mac48Address::Allocate ());
                netDevice->SetChannel (channel);
                netDevice->SetRoutingDirection (NocRoutingProtocol::BACK, 1); // SOUTH
                m_devices.Add (netDevice);
                // attach input buffering (we don't use output buffering for the moment)
                Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                netDevice->SetInQueue (inQueue);
                nocNode->AddDevice (netDevice);
                nocNode->GetRouter ()->AddDevice (netDevice);
                columnChannels[j] = channel;
              }
            else
              {
                columnChannels[j] = 0;
              }
          }
      }

    NS_LOG_DEBUG ("Printing the 2D mesh topology for the Irvine architecture (channels <-> net devices <-> nodes)...");
    for (uint32_t i = 0; i < m_devices.GetN (); ++i)
      {
        Ptr<NetDevice> device = m_devices.Get (i);
        NS_LOG_DEBUG ("\tNode " << device->GetNode()->GetId() <<
            " has a net device with (MAC) address " << device->GetAddress() <<
            " connected to channel " << device->GetChannel()->GetId());
      }
    NS_LOG_DEBUG ("Done with printing the 2D mesh topology for the Irvine architecture.");

    return m_devices;
  }

  vector<uint8_t>
  NocIrvineMesh2D::GetDestinationRelativeDimensionalPosition (uint32_t sourceNodeId, uint32_t destinationNodeId)
  {
    NS_LOG_FUNCTION (sourceNodeId << destinationNodeId);

    vector<uint8_t> relativePositions;

    uint8_t sourceX = sourceNodeId % m_hSize;
    uint8_t sourceY = sourceNodeId / m_hSize;
    uint8_t destinationX = destinationNodeId % m_hSize;
    uint8_t destinationY = destinationNodeId / m_hSize;
    uint8_t relativeX = 0;
    uint8_t relativeY = 0;
    if (destinationX < sourceX)
      {
        // 0 = East; 1 = West
        relativeX = NocHeader::DIRECTION_BIT_MASK;
      }
    if (destinationY < sourceY)
      {
        // 0 = South; 1 = North
        relativeY = NocHeader::DIRECTION_BIT_MASK;
      }
    relativeX = relativeX | std::abs ((int) (destinationX - sourceX));
    NS_LOG_DEBUG ("relativeX " << relativeX);
    relativeY = relativeY | std::abs ((int) (destinationY - sourceY));
    NS_LOG_DEBUG ("relativeY " << relativeY);

    relativePositions.insert (relativePositions.end (), relativeX);
    relativePositions.insert (relativePositions.end (), relativeY);

    return relativePositions;
  }

  void
  NocIrvineMesh2D::SaveTopology (NodeContainer nodes, string directoryPath)
    {
      NS_LOG_FUNCTION (directoryPath);

      stringstream ss;
      ss << directoryPath << FILE_SEPARATOR << m_hSize << "x" << nodes.GetN () / m_hSize << FILE_SEPARATOR << "nodes";
      string nodesXmlDirectoryPath = ss.str ();
      FileUtils::MkdirRecursive (nodesXmlDirectoryPath.c_str ());

      // saving the nodes
      for (uint32_t i = 0; i < nodes.GetN (); ++i)
        {
          Ptr<Node> node = nodes.Get (i);
          uint32_t nodeId = node->GetId ();

          ofstream nodeXml;
          ss.str ("");
          ss << nodesXmlDirectoryPath << FILE_SEPARATOR << "node-" << i << ".xml";
          string nodeXmlFilePath = ss.str ();
          NS_LOG_DEBUG ("Saving node " << nodeId << " in " << nodeXmlFilePath);
          nodeXml.open (nodeXmlFilePath.c_str (), ios::out);

          ss.str ("");
          ss << i;
          string nodeIdAsString = ss.str ();
          try
            {
              nodeType::id_type id (nodeIdAsString);
              nodeType theNode (id);
              nodeType::topologyParameter_type::topology_type topology ("mesh2D");
              nodeType::topologyParameter_type::type_type rowType ("row");
              ss.str ("");
              ss << i / m_hSize;
              string rowAsString = ss.str ();
              nodeType::topologyParameter_type::value_type rowValue (rowAsString);
              nodeType::topologyParameter_type rowTopologyParameter (topology);
              rowTopologyParameter.type (rowType);
              rowTopologyParameter.value (rowValue);
              NS_LOG_DEBUG ("Setting row topology parameter to " << rowAsString);

              nodeType::topologyParameter_type::type_type columnType ("column");
              ss.str ("");
              ss << i % m_hSize;
              string columnAsString = ss.str ();
              nodeType::topologyParameter_type::value_type columnValue (columnAsString);
              nodeType::topologyParameter_type columnTopologyParameter (topology);
              columnTopologyParameter.type (columnType);
              columnTopologyParameter.value (columnValue);
              NS_LOG_DEBUG ("Setting column topology parameter to " << columnAsString);

              theNode.topologyParameter ().push_back (rowTopologyParameter);
              theNode.topologyParameter ().push_back (columnTopologyParameter);

              for (uint32_t j = 0; j < node->GetNDevices (); ++j)
                {
                  Ptr<NetDevice> netDevice = node->GetDevice (j);
                  NS_ASSERT (netDevice != 0);NS_LOG_DEBUG ("Node " << i << " has net device " << netDevice->GetAddress ());
                  Ptr<Channel> channel = netDevice->GetChannel ();
                  if (channel == 0)
                    {
                      NS_LOG_DEBUG ("Skipping this net device because it does not have any channel attached to it");
                    }
                  else
                    {
                      uint32_t channelId = channel->GetId ();
                      NS_LOG_DEBUG ("This net device has channel " << channelId);

                      nodeType::link_type::type_type outChannelType ("out");
                      nodeType::link_type outLink (outChannelType);
                      ss.str ("");
                      ss << channelId;
                      string channelIdAsString = ss.str ();
                      nodeType::link_type::value_type outChannelValue (channelIdAsString);
                      outLink.value (outChannelValue);
                      theNode.link ().push_back (outLink);
                      NS_LOG_DEBUG ("Channel " << channelId << " was added as out link to node " << i);

                      nodeType::link_type::type_type inChannelType ("in");
                      nodeType::link_type inLink (inChannelType);
                      nodeType::link_type::value_type inChannelValue (channelIdAsString);
                      inLink.value (inChannelValue);
                      theNode.link ().push_back (inLink);
                      NS_LOG_DEBUG ("Channel " << channelId << " was added as in link to node " << i);
                    }
                }

              research::noc::application_mapping::unified_framework::schema::node::node (nodeXml, theNode);
            }
          catch (const xml_schema::exception& e)
            {
              NS_LOG_ERROR (e);
            }

          nodeXml.close ();
        }

      ss.str ("");
      ss << directoryPath << FILE_SEPARATOR << m_hSize << "x" << nodes.GetN () / m_hSize << FILE_SEPARATOR << "links";
      string linksXmlDirectoryPath = ss.str ();
      FileUtils::MkdirRecursive (linksXmlDirectoryPath.c_str ());

      list<uint32_t> visitiedChannels;

      // saving the links
      for (uint32_t i = 0; i < m_devices.GetN (); ++i)
        {
          Ptr<NetDevice> netDevice = m_devices.Get (i);
          Ptr<Channel> channel = netDevice->GetChannel ();
          if (channel != 0)
            {
              uint32_t channelId = channel->GetId ();
              bool found = false;
              for (list<uint32_t>::iterator it = visitiedChannels.begin (); it != visitiedChannels.end (); it++)
                {
                  if (*it == channelId)
                    {
                      found = true;
                      break;
                    }
                }
              if (!found)
                {
                  ofstream linkXml;
                  ss.str ("");
                  ss << linksXmlDirectoryPath << FILE_SEPARATOR << "link-" << channelId << ".xml";
                  string linkXmlFilePath = ss.str ();
                  NS_LOG_DEBUG ("Saving link " << channelId << " in " << linkXmlFilePath);
                  linkXml.open (linkXmlFilePath.c_str (), ios::out);

                  try
                    {
                      ss.str ("");
                      ss << channelId;
                      string channelIdAsString = ss.str ();
                      research::noc::application_mapping::unified_framework::schema::link::linkType::id_type id (
                          channelIdAsString);

                      NS_ASSERT_MSG (channel->GetNDevices () == 2, "Each channel is expected to connect exactly two nodes!");
                      ss.str ("");
                      ss << channel->GetDevice (0)->GetNode ()->GetId ();
                      string firstNodeIdAsString = ss.str ();
                      research::noc::application_mapping::unified_framework::schema::link::linkType::firstNode_type firstNode (
                          firstNodeIdAsString);
                      ss.str ("");
                      ss << channel->GetDevice (1)->GetNode ()->GetId ();
                      string secondNodeIdAsString = ss.str ();
                      research::noc::application_mapping::unified_framework::schema::link::linkType::firstNode_type secondNode (
                          secondNodeIdAsString);

                      NS_LOG_DEBUG ("Link " << channelId << " has as first node " << firstNodeIdAsString << " and second node " << secondNodeIdAsString);
                      research::noc::application_mapping::unified_framework::schema::link::linkType theLink (firstNode,
                          secondNode, id);

                      research::noc::application_mapping::unified_framework::schema::link::link (linkXml, theLink);
                    }
                  catch (const xml_schema::exception& e)
                    {
                      NS_LOG_ERROR (e);
                    }

                  linkXml.close ();
                  visitiedChannels.push_back (channelId);
                }
            }

        }
    }

} // namespace ns3
