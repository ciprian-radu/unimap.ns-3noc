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

#include "noc-helper.h"
#include "ns3/config.h"
#include "ns3/noc-routing-protocol.h"
#include "ns3/xy-routing.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("NocHelper");

namespace ns3
{

  NocHelper::NocHelper ()
  {
    m_channelFactory.SetTypeId ("ns3::NocChannel");
    m_inQueueFactory.SetTypeId ("ns3::DropTailQueue");
    m_outQueueFactory.SetTypeId ("ns3::DropTailQueue");
  }

  void
  NocHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
  {
    m_channelFactory.Set (n1, v1);
  }

  void
  NocHelper::SetInQueue (std::string type,
                         std::string n1, const AttributeValue &v1,
                         std::string n2, const AttributeValue &v2,
                         std::string n3, const AttributeValue &v3,
                         std::string n4, const AttributeValue &v4)
  {
    m_inQueueFactory.SetTypeId (type);
    m_inQueueFactory.Set (n1, v1);
    m_inQueueFactory.Set (n2, v2);
    m_inQueueFactory.Set (n3, v3);
    m_inQueueFactory.Set (n4, v4);
  }

  void
  NocHelper::SetOutQueue (std::string type,
                          std::string n1, const AttributeValue &v1,
                          std::string n2, const AttributeValue &v2,
                          std::string n3, const AttributeValue &v3,
                          std::string n4, const AttributeValue &v4)
  {
    m_outQueueFactory.SetTypeId (type);
    m_outQueueFactory.Set (n1, v1);
    m_outQueueFactory.Set (n2, v2);
    m_outQueueFactory.Set (n3, v3);
    m_outQueueFactory.Set (n4, v4);
  }

  void
  NocHelper::EnableAscii(std::ostream &os, uint32_t nodeid, uint32_t deviceid)
  {
    Ptr<AsciiWriter> writer = AsciiWriter::Get(os);
    Packet::EnablePrinting();
    std::ostringstream oss;

    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::NocNetDevice/Send";
    Config::Connect(oss.str(), MakeBoundCallback(&NocHelper::AsciiTxEvent, writer));
    oss.str ("");

    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::NocNetDevice/Receive";
    Config::Connect(oss.str(), MakeBoundCallback(&NocHelper::AsciiRxEvent, writer));
  }

  void
  NocHelper::EnableAscii(std::ostream &os, NetDeviceContainer d)
  {
    for (NetDeviceContainer::Iterator i = d.Begin(); i != d.End(); ++i)
      {
        Ptr<NetDevice> dev = *i;
        EnableAscii(os, dev->GetNode()->GetId(), dev->GetIfIndex());
      }
  }

  void
  NocHelper::EnableAscii(std::ostream &os, NodeContainer n)
  {
    NetDeviceContainer devs;
    for (NodeContainer::Iterator i = n.Begin(); i != n.End(); ++i)
      {
        Ptr<Node> node = *i;
        for (uint32_t j = 0; j < node->GetNDevices(); ++j)
          {
            devs.Add(node->GetDevice(j));
          }
      }
    EnableAscii(os, devs);
  }

  void
  NocHelper::EnableAsciiAll(std::ostream &os)
  {
    EnableAscii(os, NodeContainer::GetGlobal());
  }

  NetDeviceContainer
  NocHelper::Install(NodeContainer nodes, Ptr<NocChannel> channel)
  {
    for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
      {
        Ptr<NocNetDevice> dev = CreateObject<NocNetDevice> ();
        dev->SetAddress(Mac48Address::Allocate());
        dev->SetChannel(channel);
        Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
        dev->SetInQueue (inQueue);
        (*i)->AddDevice(dev);

        Ptr<NocNode> nocNode = (*i)->GetObject<NocNode> ();
        if (nocNode->GetRoutingProtocol () == 0)
          {
            Ptr<NocRoutingProtocol> routingProtocol =
                CreateObject<XyRouting> ();
            routingProtocol->SetNocNode(nocNode);
            nocNode->SetRoutingProtocol(routingProtocol);
          }

        m_devices.Add(dev);
      }
    return m_devices;
  }

  NetDeviceContainer
  NocHelper::Install(NodeContainer nodes)
  {
    Ptr<NocChannel> channel = m_channelFactory.Create ()->GetObject<NocChannel> ();
    return Install(nodes, channel);
  }

  NetDeviceContainer
  NocHelper::Install2DMesh(NodeContainer nodes, uint32_t hSize)
  {
    Ptr<NocChannel> channel = 0;
    Ptr<NocNetDevice> netDevice;

    for (unsigned int i = 0; i < nodes.GetN(); ++i)
      {
        Ptr<NocNode> nocNode = nodes.Get (i)->GetObject<NocNode> ();
        if (nocNode->GetRoutingProtocol() == 0)
          {
            Ptr<NocRoutingProtocol> routingProtocol =
                CreateObject<XyRouting> ();
            routingProtocol->SetNocNode(nocNode);
            nocNode->SetRoutingProtocol(routingProtocol);
          }
      }

    // create the horizontal channels (and net devices)
    for (unsigned int i = 0; i < nodes.GetN(); ++i)
      {
        if (channel != 0)
          {
            netDevice = CreateObject<NocNetDevice> ();
            netDevice->SetAddress(Mac48Address::Allocate());
            netDevice->SetChannel(channel);
            netDevice->SetRoutingDirection(XyRouting::WEST);
            Ptr<NocRoutingProtocol> routingProtocol = CreateObject<XyRouting> ();
            m_devices.Add(netDevice);
            netDevice->SetNocHelper (this);
            // attach input buffering (we don't use output buffering for the moment)
            Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
            netDevice->SetInQueue (inQueue);
            nodes.Get(i)->AddDevice(netDevice);
          }

        if (i == 0 || (i > 0 && (i + 1) % hSize != 0))
          {
            channel = m_channelFactory.Create ()->GetObject<NocChannel> ();
            netDevice = CreateObject<NocNetDevice> ();
            netDevice->SetAddress(Mac48Address::Allocate());
            netDevice->SetChannel(channel);
            netDevice->SetRoutingDirection(XyRouting::EAST);
            Ptr<NocRoutingProtocol> routingProtocol = CreateObject<XyRouting> ();
            m_devices.Add(netDevice);
            netDevice->SetNocHelper (this);
            // attach input buffering (we don't use output buffering for the moment)
            Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
            netDevice->SetInQueue (inQueue);
            nodes.Get(i)->AddDevice(netDevice);
          }
        else
          {
            channel = 0;
          }
      }

    // create the vertical channels (and net devices)
    channel = 0;
    std::vector< Ptr<NocChannel> > columnChannels(hSize);
    for (unsigned int i = 0; i < nodes.GetN(); i = i + hSize)
      {
        for (unsigned int j = 0; j < hSize; ++j)
          {
            if (columnChannels[j] != 0)
              {
                channel = columnChannels[j];
                netDevice = CreateObject<NocNetDevice> ();
                netDevice->SetAddress(Mac48Address::Allocate());
                netDevice->SetChannel(channel);
                netDevice->SetRoutingDirection(XyRouting::NORTH);
                Ptr<NocRoutingProtocol> routingProtocol = CreateObject<XyRouting> ();
                m_devices.Add(netDevice);
                netDevice->SetNocHelper (this);
                // attach input buffering (we don't use output buffering for the moment)
                Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                netDevice->SetInQueue (inQueue);
                nodes.Get(i + j)->AddDevice(netDevice);
              }
            if (i < nodes.GetN() - hSize)
              {
                channel = m_channelFactory.Create ()->GetObject<NocChannel> ();
                netDevice = CreateObject<NocNetDevice> ();
                netDevice->SetAddress(Mac48Address::Allocate());
                netDevice->SetChannel(channel);
                netDevice->SetRoutingDirection(XyRouting::SOUTH);
                Ptr<NocRoutingProtocol> routingProtocol = CreateObject<XyRouting> ();
                m_devices.Add(netDevice);
                netDevice->SetNocHelper (this);
                // attach input buffering (we don't use output buffering for the moment)
                Ptr<Queue> inQueue = m_inQueueFactory.Create<Queue> ();
                netDevice->SetInQueue (inQueue);
                nodes.Get(i + j)->AddDevice(netDevice);
                columnChannels[j] = channel;
              }
            else
              {
                columnChannels[j] = 0;
              }
          }
      }

    NS_LOG_DEBUG ("Printing the 2D mesh topology (channels <-> net devices <-> nodes)...");
    for (uint32_t i = 0; i < m_devices.GetN(); ++i)
      {
        Ptr<NetDevice> device = m_devices.Get(i);
        NS_LOG_DEBUG ("\tNode " << device->GetNode()->GetId() <<
            " has a net device with (MAC) address " << device->GetAddress() <<
            " connected to channel " << device->GetChannel()->GetId());
      }
    NS_LOG_DEBUG ("Done with printing the 2D mesh topology.");

    return m_devices;
  }

  Ptr<NocNetDevice>
  NocHelper::FindNetDeviceByAddress (Mac48Address address)
  {
    Ptr<NocNetDevice> nocNetDevice = 0;
    for (unsigned int i = 0; i < m_devices.GetN(); ++i) {
      Ptr<NetDevice> netDevice = m_devices.Get (i);
      Mac48Address macAddress = Mac48Address::ConvertFrom(netDevice->GetAddress ());
      if (macAddress == address)
        {
          nocNetDevice = netDevice->GetObject<NocNetDevice> ();
        }
    }
    return nocNetDevice;
  }

  void
  NocHelper::AsciiTxEvent(Ptr<AsciiWriter> writer, std::string path, Ptr<
      const Packet> packet)
  {
    writer->WritePacket(AsciiWriter::TX, path, packet);
  }

  void
  NocHelper::AsciiRxEvent(Ptr<AsciiWriter> writer, std::string path, Ptr<
      const Packet> packet)
  {
    writer->WritePacket(AsciiWriter::RX, path, packet);
  }

  void
  NocHelper::AsciiEnqueueEvent(Ptr<AsciiWriter> writer, std::string path, Ptr<
      const Packet> packet)
  {
    writer->WritePacket(AsciiWriter::ENQUEUE, path, packet);
  }

  void
  NocHelper::AsciiDequeueEvent(Ptr<AsciiWriter> writer, std::string path, Ptr<
      const Packet> packet)
  {
    writer->WritePacket(AsciiWriter::DEQUEUE, path, packet);
  }

  void
  NocHelper::AsciiDropEvent(Ptr<AsciiWriter> writer, std::string path, Ptr<
      const Packet> packet)
  {
    writer->WritePacket(AsciiWriter::DROP, path, packet);
  }

} // namespace ns3

