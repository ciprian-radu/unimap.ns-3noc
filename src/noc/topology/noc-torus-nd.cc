/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 - 2011
 *               Advanced Computer Architecture and Processing Systems (ACAPS),
 *               Lucian Blaga University of Sibiu, Romania
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
 * Author: Andreea Gancea <andreea.gancea@ulbsibiu.ro>
 *
 */

#include "noc-torus-nd.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/xyz-routing.h"
#include "ns3/4-way-router.h"
#include "ns3/irvine-router.h"
#include "ns3/saf-switching.h"
#include "ns3/wormhole-switching.h"
#include "ns3/vct-switching.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/uinteger.h"
#include "ns3/file-utils.h"
#include "ns3/object-vector.h"
#include "ns3/noc-registry.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "noc-value.h"


NS_LOG_COMPONENT_DEFINE ("NocTorusND");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED ( NocTorusND);

  TypeId
  NocTorusND::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::NocTorusND")
        .SetParent<NocTopology> ()
        .AddConstructor<NocTorusND> ()
        .AddAttribute ("Size",
              "how many nodes the nD Torus will have on each line",
              ObjectVectorValue(),
              MakeObjectVectorAccessor (&NocTorusND::m_size),
              MakeObjectVectorChecker<NocValue> ())
     ;
    return tid;
  }

  NocTorusND::NocTorusND () : NocTopology ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  NocTorusND::NocTorusND (vector<Ptr<NocValue> > size) : NocTopology ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_size = size;
  }

  NocTorusND::~NocTorusND ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  uint32_t
  NocTorusND::GetNumberOfNodes (int32_t dimension)
  {
    uint32_t numberOfNodes;
    numberOfNodes = 1;

    for (int i = 0; i < dimension; i++)
      {
        if (m_size.at(i)!= 0)
          {
            numberOfNodes = numberOfNodes * m_size.at (i)->GetValue ();
          }
      }

    return numberOfNodes;

  }

  NetDeviceContainer
  NocTorusND::Install (NodeContainer nodes)
  {
    NS_LOG_FUNCTION_NOARGS ();

    for (unsigned int i = 0; i < m_size.size (); i++)
      {
        NS_LOG_DEBUG ("Dimension " << (i+1) << " size: " << m_size.at (i)->GetValue ());
      }

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
            Ptr<NocRoutingProtocol> routingProtocol = m_routingProtocolFactory.Create ()->GetObject<NocRoutingProtocol> ();
            router->SetRoutingProtocol (routingProtocol);
            Ptr<NocSwitchingProtocol> switchingProtocol =
                m_switchingProtocolFactory.Create ()->GetObject<NocSwitchingProtocol> ();
            router->SetSwitchingProtocol (switchingProtocol);
          }
      }

    // create the channels (and net devices)
    for (unsigned int k = 0; k < m_size.size (); k++)
      {
        channel = 0;
        std::vector<Ptr<NocChannel> > channels (GetNumberOfNodes (k));
        std::vector<Ptr<NocChannel> > channels_torus (GetNumberOfNodes (k));

        for (unsigned int i = 0; i < nodes.GetN (); i = i + GetNumberOfNodes (k))
          {
            for (unsigned int j = 0; j < GetNumberOfNodes (k); j++)
              {

                Ptr<NocNode> nocNode = nodes.Get (i + j)->GetObject<NocNode> ();

                if (channels[j] != 0)
                  {
                    channel = channels[j];
                    netDevice = CreateObject<NocNetDevice> ();
                    netDevice->SetAddress (Mac48Address::Allocate ());
                    netDevice->SetChannel (channel);
                    netDevice->SetRoutingDirection (NocRoutingProtocol::BACK, k);
                    m_devices.Add (netDevice);
                    // attach input buffering (we don't use output buffering for the moment)
                    Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                    netDevice->SetInQueue (inQueue);
                    nocNode->AddDevice (netDevice);
                    nocNode->GetRouter ()->AddDevice (netDevice);
                  }
                if (((i + j) % GetNumberOfNodes (k+1)) < (GetNumberOfNodes (k+1) - GetNumberOfNodes (k)))
                  {
                    channel = m_channelFactory.Create ()->GetObject<NocChannel> ();
                    netDevice = CreateObject<NocNetDevice> ();
                    netDevice->SetAddress (Mac48Address::Allocate ());
                    netDevice->SetChannel (channel);
                    netDevice->SetRoutingDirection (NocRoutingProtocol::FORWARD, k);
                    m_devices.Add (netDevice);
                    // attach input buffering (we don't use output buffering for the moment)
                    Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                    netDevice->SetInQueue (inQueue);
                    nocNode->AddDevice (netDevice);
                    nocNode->GetRouter ()->AddDevice (netDevice);
                    channels[j] = channel;
                  }
                else
                  {
                    channels[j] = 0;
                  }
                if (((i + j) % GetNumberOfNodes (k+1)) <  GetNumberOfNodes (k))
                  {
                    channel = m_channelFactory.Create ()->GetObject<NocChannel> ();
                    netDevice = CreateObject<NocNetDevice> ();
                    netDevice->SetAddress (Mac48Address::Allocate ());
                    netDevice->SetChannel (channel);
                    netDevice->SetRoutingDirection (NocRoutingProtocol::BACK, k);
                    m_devices.Add (netDevice);
                    // attach input buffering (we don't use output buffering for the moment)
                    Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                    netDevice->SetInQueue (inQueue);
                    nocNode->AddDevice (netDevice);
                    nocNode->GetRouter ()->AddDevice (netDevice);
                    channels_torus[j] = channel;
                  }
                if (((i + j) % GetNumberOfNodes (k + 1)) >= (GetNumberOfNodes (k + 1) - GetNumberOfNodes (k)))
                  {
                    channel = channels_torus[j];
                    netDevice = CreateObject<NocNetDevice> ();
                    netDevice->SetAddress (Mac48Address::Allocate ());
                    netDevice->SetChannel (channel);
                    netDevice->SetRoutingDirection (NocRoutingProtocol::FORWARD, k);
                    m_devices.Add (netDevice);
                    // attach input buffering (we don't use output buffering for the moment)
                    Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                    netDevice->SetInQueue (inQueue);
                    nocNode->AddDevice (netDevice);
                    nocNode->GetRouter ()->AddDevice (netDevice);
                  }
              }
          }
      }

    NS_LOG_DEBUG ("Printing the " << m_size.size() << "D torus topology (channels <-> net devices <-> nodes)...");
    for (uint32_t i = 0; i < m_devices.GetN (); ++i)
      {
        Ptr<NetDevice> device = m_devices.Get (i);
        NS_LOG_DEBUG ("\tNode " << device->GetNode ()->GetId () << " has a net device with (MAC) address "
            << device->GetAddress () << " connected to channel " << device->GetChannel ()->GetId ());
      } NS_LOG_DEBUG ("Done with printing the " << m_size.size() << "D torus topology.");

    return m_devices;
  }

  vector<uint8_t>
  NocTorusND::GetDestinationRelativeDimensionalPosition (uint32_t sourceNodeId, uint32_t destinationNodeId)
  {
    NS_LOG_FUNCTION (sourceNodeId << destinationNodeId);

    vector<uint8_t> relativePositions = vector<uint8_t> (m_size.size());
    vector<uint32_t> source = vector<uint32_t> (m_size.size());

    for (unsigned int k = 0; k < m_size.size(); k++)
      {
        source.at (k) = (sourceNodeId / GetNumberOfNodes (k)) % m_size.at (k)->GetValue ();
        NS_LOG_DEBUG ("dimension " << (k+1) << " source " << source.at(k));
      }

    vector<uint32_t> destination = vector<uint32_t> (m_size.size());
    //uint32_t g=0;
    for (unsigned int k = 0; k < m_size.size(); k++)
      {
        //g=GetNumberOfNodes (k);
        destination.at (k) = (destinationNodeId / GetNumberOfNodes (k)) % m_size.at (k)->GetValue ();
        NS_LOG_DEBUG ("dimension " << (k+1) << " destination " << destination.at(k));
      }

    vector<uint8_t> relative = vector<uint8_t> (m_size.size(), 0);

    for (unsigned int k = 0; k < m_size.size (); k++)
      {
        int offset = ((int) (destination.at (k) - source.at (k))) >= 0 ? (destination.at (k) - source.at (k))
            % m_size.at (k)->GetValue () : (m_size.at (k)->GetValue () + destination.at (k) - source.at (k))
            % m_size.at (k)->GetValue ();
        if (offset > (int) (m_size.at (k)->GetValue() / 2))
          {
            offset = offset - m_size.at (k)->GetValue();
          }
        NS_LOG_DEBUG ("dimension " << (k+1) << " offset " << offset);

        if (offset < 0)
          {
            // 0 = East; 1 = West
            relative.at (k) = NocHeader::DIRECTION_BIT_MASK;
            offset = std::abs (offset);
          }
        relative.at (k) = relative.at (k) | offset;
        NS_LOG_DEBUG ("dimension "<<(k+1)<<" relative " <<(int) relative.at(k));
        relativePositions.at (k) = relative.at (k);
      }

    return relativePositions;
  }

  void
  NocTorusND::SaveTopology (NodeContainer nodes, string directoryPath)
  {
    NS_LOG_FUNCTION (directoryPath);

    stringstream ss;

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
            nodeType::topologyParameter_type::topology_type topology ("TorusND");
            for (unsigned int k = 0; k < GetNumberOfNodes (m_size.size ()); k++)
              {
                std::stringstream s;
                s << k;
                nodeType::topologyParameter_type::type_type dimensionType (s.str());
                ss.str ("");
                ss << i / GetNumberOfNodes (k - 1) % m_size.at (k)->GetValue ();
                string dimensionAsString = ss.str ();
                nodeType::topologyParameter_type::value_type dimensionValue (dimensionAsString);
                nodeType::topologyParameter_type dimensionTopologyParameter (topology);
                dimensionTopologyParameter.type (dimensionType);
                dimensionTopologyParameter.value (dimensionValue);
                NS_LOG_DEBUG ("Setting dimension " << (k+1) << " topology parameter to " << dimensionAsString);
                theNode.topologyParameter ().push_back (dimensionTopologyParameter);
              }

            for (uint32_t j = 0; j < node->GetNDevices (); ++j)
              {
                Ptr<NetDevice> netDevice = node->GetDevice (j);
                NS_ASSERT (netDevice != 0);
                NS_LOG_DEBUG ("Node " << i << " has net device " << netDevice->GetAddress ());
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

    /*ss.str ("");
    ss << directoryPath << FILE_SEPARATOR << m_size << "x" << nodes.GetN () / m_size << FILE_SEPARATOR << "links";*/
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

                    NS_LOG_DEBUG ("Link " << channelId << " has as first node " << firstNodeIdAsString << " and second node "
                        << secondNodeIdAsString);
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

