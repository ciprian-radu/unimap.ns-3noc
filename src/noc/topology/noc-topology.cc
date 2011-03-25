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
 * Author: Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *         http://webspace.ulbsibiu.ro/ciprian.radu/
 */

#include "noc-topology.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/xy-routing.h"
#include "ns3/xyz-routing.h"
#include "ns3/4-way-router.h"
#include "ns3/irvine-router.h"
#include "ns3/saf-switching.h"
#include "ns3/wormhole-switching.h"
#include "ns3/vct-switching.h"
#include "ns3/noc-packet-tag.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("NocTopology");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocTopology);

  TypeId
  NocTopology::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::NocTopology")
        .SetParent<Object> ();
    return tid;
  }

  NocTopology::NocTopology ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_channelFactory.SetTypeId ("ns3::NocChannel");
    m_inQueueFactory.SetTypeId ("ns3::DropTailQueue");
    m_outQueueFactory.SetTypeId ("ns3::DropTailQueue");
  }

  NocTopology::~NocTopology ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NocTopology::SetChannelAttribute (std::string n1, const AttributeValue &v1)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_channelFactory.Set (n1, v1);
  }

  void
  NocTopology::SetInQueue (std::string type, std::string n1, const AttributeValue &v1, std::string n2,
      const AttributeValue &v2, std::string n3, const AttributeValue &v3, std::string n4, const AttributeValue &v4)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_inQueueFactory.SetTypeId (type);
    m_inQueueFactory.Set (n1, v1);
    m_inQueueFactory.Set (n2, v2);
    m_inQueueFactory.Set (n3, v3);
    m_inQueueFactory.Set (n4, v4);
  }

  void
  NocTopology::SetOutQueue (std::string type, std::string n1, const AttributeValue &v1, std::string n2,
      const AttributeValue &v2, std::string n3, const AttributeValue &v3, std::string n4, const AttributeValue &v4)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_outQueueFactory.SetTypeId (type);
    m_outQueueFactory.Set (n1, v1);
    m_outQueueFactory.Set (n2, v2);
    m_outQueueFactory.Set (n3, v3);
    m_outQueueFactory.Set (n4, v4);
  }

  void
  NocTopology::SetRouter (string type)
  {
    NS_LOG_FUNCTION (type);

    m_routerFactory.SetTypeId (type);
    // the assert below doesn't work if the router doesn't descend *directly* from NocRouter
//    NS_ASSERT_MSG (NocRouter::GetTypeId () == m_routerFactory.GetTypeId ().GetParent (), "The router type " << type << " is not a child of " << NocRouter::GetTypeId ());
  }

  void
  NocTopology::SetRouterAttribute (string attributeName, const AttributeValue &attributeValue)
  {
    NS_LOG_FUNCTION (attributeName << &attributeValue);

    m_routerFactory.Set (attributeName, attributeValue);
  }

  void
  NocTopology::SetRoutingProtocol (string type)
  {
    NS_LOG_FUNCTION (type);

    m_routingProtocolFactory.SetTypeId (type);
    NS_ASSERT_MSG (NocRoutingProtocol::GetTypeId () == m_routingProtocolFactory.GetTypeId ().GetParent (), "The routing protocol type " << type << " is not a child of " << NocRoutingProtocol::GetTypeId ());
  }

  void
  NocTopology::SetRoutingProtocolAttribute (string attributeName, const AttributeValue &attributeValue)
  {
    NS_LOG_FUNCTION (attributeName << &attributeValue);

    m_routingProtocolFactory.Set (attributeName, attributeValue);
  }

  void
  NocTopology::SetSwitchingProtocol (string type)
  {
    NS_LOG_FUNCTION (type);

    m_switchingProtocolFactory.SetTypeId (type);
    NS_ASSERT_MSG (NocSwitchingProtocol::GetTypeId () == m_switchingProtocolFactory.GetTypeId ().GetParent (), "The switching protocol type " << type << " is not a child of " << NocSwitchingProtocol::GetTypeId ());
  }

  void
  NocTopology::SetSwitchingProtocolAttribute (string attributeName, const AttributeValue &attributeValue)
  {
    NS_LOG_FUNCTION (attributeName << &attributeValue);

    m_switchingProtocolFactory.Set (attributeName, attributeValue);
  }

  double
  NocTopology::GetDynamicPower ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    double power = 0;

    map<uint32_t, bool> processedChannels;
    for (uint32_t i = 0; i < m_devices.GetN (); i++)
      {
        Ptr<NocNetDevice> device = m_devices.Get (i)->GetObject<NocNetDevice> ();
        Ptr<Channel> channel = device->GetChannel ();
        if (channel != 0)
          {
            Ptr<NocChannel> nocChannel = channel->GetObject<NocChannel> ();
            uint32_t channelId = nocChannel->GetId ();
            if (!processedChannels[channelId])
              {
                double channelDynamicPower = nocChannel->GetDynamicPower ();
                NS_LOG_DEBUG ("Channel " << channelId << " consumed a dynamic power of " << channelDynamicPower << " W");
                power += channelDynamicPower;
                processedChannels[channelId] = true;
              }
          }
      }
    for (uint32_t i = 0; i < m_nodes.GetN (); i++)
      {
        Ptr<NocNode> nocNode = m_nodes.Get (i)->GetObject<NocNode> ();
        Ptr<NocRouter> router = nocNode->GetRouter ();
        double routerPower = router->GetDynamicPower ();
        NS_LOG_DEBUG ("Router " << nocNode->GetId () << " consumed a dynamic power of " << routerPower << " W");
        power += routerPower;
      }

    NS_LOG_DEBUG ("NoC dynamic power is " << power << " W");
    return power;
  }

  double
  NocTopology::GetLeakagePower ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    double power = 0;

    map<uint32_t, bool> processedChannels;
    for (uint32_t i = 0; i < m_devices.GetN (); i++)
      {
        Ptr<NocNetDevice> device = m_devices.Get (i)->GetObject<NocNetDevice> ();
        Ptr<Channel> channel = device->GetChannel ();
        if (channel != 0)
          {
            Ptr<NocChannel> nocChannel = channel->GetObject<NocChannel> ();
            uint32_t channelId = nocChannel->GetId ();
            if (!processedChannels[channelId])
              {
                double channelLeakagePower = nocChannel->GetLeakagePower ();
                NS_LOG_DEBUG ("Channel " << channelId << " consumed a leakage power of " << channelLeakagePower << " W");
                power += channelLeakagePower;
                processedChannels[channelId] = true;
              }
          }
      }
    for (uint32_t i = 0; i < m_nodes.GetN (); i++)
      {
        Ptr<NocNode> nocNode = m_nodes.Get (i)->GetObject<NocNode> ();
        Ptr<NocRouter> router = nocNode->GetRouter ();
        double routerPower = router->GetLeakagePower ();
        NS_LOG_DEBUG ("Router " << nocNode->GetId () << " consumed a leakage power of " << routerPower << " W");
        power += routerPower;
      }

    NS_LOG_DEBUG ("NoC leakage power is " << power << " W");
    return power;
  }

  double
  NocTopology::GetTotalPower ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    double power = 0;

    map<uint32_t, bool> processedChannels;
    for (uint32_t i = 0; i < m_devices.GetN (); i++)
      {
        Ptr<NocNetDevice> device = m_devices.Get (i)->GetObject<NocNetDevice> ();
        Ptr<Channel> channel = device->GetChannel ();
        if (channel != 0)
          {
            Ptr<NocChannel> nocChannel = channel->GetObject<NocChannel> ();
            uint32_t channelId = nocChannel->GetId ();
            if (!processedChannels[channelId])
              {
                double channelTotalPower = nocChannel->GetTotalPower ();
                NS_LOG_DEBUG ("Channel " << channelId << " consumed a total power of " << channelTotalPower << " W");
                power += channelTotalPower;
                processedChannels[channelId] = true;
              }
          }
      }
    for (uint32_t i = 0; i < m_nodes.GetN (); i++)
      {
        Ptr<NocNode> nocNode = m_nodes.Get (i)->GetObject<NocNode> ();
        Ptr<NocRouter> router = nocNode->GetRouter ();
        double routerPower = router->GetTotalPower ();
        NS_LOG_DEBUG ("Router " << nocNode->GetId () << " consumed a total power of " << routerPower << " W");
        power += routerPower;
      }

    NS_LOG_DEBUG ("NoC total power is " << power << " W");
    return power;
  }

  double
  NocTopology::GetArea ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    double area = 0;

    map<uint32_t, bool> processedChannels;
    for (uint32_t i = 0; i < m_devices.GetN (); i++)
      {
        Ptr<NocNetDevice> device = m_devices.Get (i)->GetObject<NocNetDevice> ();
        Ptr<Channel> channel = device->GetChannel ();
        if (channel != 0)
          {
            Ptr<NocChannel> nocChannel = channel->GetObject<NocChannel> ();
            uint32_t channelId = nocChannel->GetId ();
            if (!processedChannels[channelId])
              {
                double channelArea = nocChannel->GetArea ();
                NS_LOG_DEBUG ("Channel " << channelId << " requires an area of " << channelArea << " um^2");
                area += channelArea;
                processedChannels[channelId] = true;
              }
          }
      }
    for (uint32_t i = 0; i < m_nodes.GetN (); i++)
      {
        Ptr<NocNode> nocNode = m_nodes.Get (i)->GetObject<NocNode> ();
        Ptr<NocRouter> router = nocNode->GetRouter ();
        double routerArea = router->GetArea ();
        NS_LOG_DEBUG ("Router " << nocNode->GetId () << " requires an area of " << routerArea << " um^2");
        area += routerArea;
      }

    NS_LOG_DEBUG ("NoC total area is " << area << " um^2");
    return area;
  }

  void
  NocTopology::EnableAscii (Ptr<OutputStreamWrapper> stream, uint32_t nodeid, uint32_t deviceid)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Packet::EnablePrinting ();
    std::ostringstream oss;

    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::NocNetDevice/Send";
    Config::Connect (oss.str (), MakeBoundCallback (&NocTopology::AsciiTxEvent, stream));
    oss.str ("");

    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::NocNetDevice/Receive";
    Config::Connect (oss.str (), MakeBoundCallback (&NocTopology::AsciiRxEvent, stream));
    oss.str ("");

    oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::NocNetDevice/Drop";
    Config::Connect (oss.str (), MakeBoundCallback (&NocTopology::AsciiDropEvent, stream));
    oss.str ("");
  }

  void
  NocTopology::EnableAscii (Ptr<OutputStreamWrapper> stream, NetDeviceContainer d)
  {
    NS_LOG_FUNCTION_NOARGS ();

    for (NetDeviceContainer::Iterator i = d.Begin (); i != d.End (); ++i)
      {
        Ptr<NetDevice> dev = *i;
        EnableAscii (stream, dev->GetNode ()->GetId (), dev->GetIfIndex ());
      }
  }

  void
  NocTopology::EnableAscii (Ptr<OutputStreamWrapper> stream, NodeContainer n)
  {
    NS_LOG_FUNCTION_NOARGS ();

    NetDeviceContainer devs;
    for (NodeContainer::Iterator i = n.Begin (); i != n.End (); ++i)
      {
        Ptr<Node> node = *i;
        for (uint32_t j = 0; j < node->GetNDevices (); ++j)
          {
            devs.Add (node->GetDevice (j));
          }
      }
    EnableAscii (stream, devs);
  }

  void
  NocTopology::EnableAsciiAll (Ptr<OutputStreamWrapper> stream)
  {
    NS_LOG_FUNCTION_NOARGS ();

    EnableAscii (stream, NodeContainer::GetGlobal ());
  }

  Ptr<NocNetDevice>
  NocTopology::FindNetDeviceByAddress (Mac48Address address)
  {
    NS_LOG_FUNCTION_NOARGS ();

    Ptr<NocNetDevice> nocNetDevice = 0;
    for (unsigned int i = 0; i < m_devices.GetN (); ++i)
      {
        Ptr<NetDevice> netDevice = m_devices.Get (i);
        Mac48Address macAddress = Mac48Address::ConvertFrom (netDevice->GetAddress ());
        if (macAddress == address)
          {
            nocNetDevice = netDevice->GetObject<NocNetDevice> ();
          }
      }

    NS_ASSERT_MSG (nocNetDevice != 0, "NoC net device with address " << address << " was not found in the topology!");

    return nocNetDevice;
  }

  void
  NocTopology::AsciiTxEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();

    *stream->GetStream () << "t " << Simulator::Now () << " " << path << " " << *packet;

    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        *stream->GetStream () << " (UID: " << (int) packet->GetUid () << ")";
      }
    if (NocPacket::TAIL == tag.GetPacketType ())
      {
        *stream->GetStream () << "(tail flit)" << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }
    if (NocPacket::DATA == tag.GetPacketType ())
      {
        *stream->GetStream ()  << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }

    *stream->GetStream () << std::endl;
  }

  void
  NocTopology::AsciiRxEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();

    *stream->GetStream () << "r " << Simulator::Now () << " " << path << " " << *packet;

    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        *stream->GetStream () << " (UID: " << (int) packet->GetUid () << ")";
      }
    if (NocPacket::TAIL == tag.GetPacketType ())
      {
        *stream->GetStream () << "(tail flit)" << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }
    if (NocPacket::DATA == tag.GetPacketType ())
      {
        *stream->GetStream ()  << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }

    *stream->GetStream () << std::endl;
  }

  void
  NocTopology::AsciiEnqueueEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();

    *stream->GetStream () << "+ " << Simulator::Now () << " " << path << " " << *packet;

    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        *stream->GetStream () << " (UID: " << (int) packet->GetUid () << ")";
      }
    if (NocPacket::TAIL == tag.GetPacketType ())
      {
        *stream->GetStream () << "(tail flit)" << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }
    if (NocPacket::DATA == tag.GetPacketType ())
      {
        *stream->GetStream ()  << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }

    *stream->GetStream () << std::endl;
  }

  void
  NocTopology::AsciiDequeueEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();

    *stream->GetStream () << "- " << Simulator::Now () << " " << path << " " << *packet;

    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        *stream->GetStream () << " (UID: " << (int) packet->GetUid () << ")";
      }
    if (NocPacket::TAIL == tag.GetPacketType ())
      {
        *stream->GetStream () << "(tail flit)" << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }
    if (NocPacket::DATA == tag.GetPacketType ())
      {
        *stream->GetStream ()  << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }

    *stream->GetStream () << std::endl;
  }

  void
  NocTopology::AsciiDropEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();

    *stream->GetStream () << "d " << Simulator::Now () << " " << path << " " << *packet;

    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        *stream->GetStream () << " (UID: " << (int) packet->GetUid () << ")";
      }
    if (NocPacket::TAIL == tag.GetPacketType ())
      {
        *stream->GetStream () << "(tail flit)" << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }
    if (NocPacket::DATA == tag.GetPacketType ())
      {
        *stream->GetStream ()  << " (head flit UID: " << (int) tag.GetPacketHeadUid () << ")";
      }

    *stream->GetStream () << std::endl;
  }

} // namespace ns3
