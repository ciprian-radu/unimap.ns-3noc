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

#include "irvine-router.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/xy-routing.h"
#include "limits.h"
#include "ns3/queue.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/uinteger.h"
#include "ns3/noc-packet-tag.h"

NS_LOG_COMPONENT_DEFINE ("IrvineRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (IrvineRouter);

  TypeId
  IrvineRouter::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::IrvineRouter")
        .SetParent<NocRouter> ()
        .AddConstructor<IrvineRouter> ();
    return tid;
  }

  // we could easily name the router "Irvine router", but using __FILE__ should be more useful for debugging
  IrvineRouter::IrvineRouter () : NocRouter (__FILE__)
  {
    Init ();
  }

  IrvineRouter::IrvineRouter (std::string name) : NocRouter (name)
  {
    Init ();
  }

  void
  IrvineRouter::Init ()
  {
    m_internalLeftInputDevice = CreateObject<NocNetDevice> ();
    m_internalLeftInputDevice->SetAddress (Mac48Address::Allocate ());

    m_internalRightInputDevice = CreateObject<NocNetDevice> ();
    m_internalRightInputDevice->SetAddress (Mac48Address::Allocate ());

    m_internalOutputDevice = CreateObject<NocNetDevice> ();
    m_internalOutputDevice->SetAddress (Mac48Address::Allocate ());

    m_north1DeviceAdded = false;
    m_north2DeviceAdded = false;
    m_eastDeviceAdded = false;
    m_south1DeviceAdded = false;
    m_south2DeviceAdded = false;
    m_westDeviceAdded = false;
  }

  IrvineRouter::~IrvineRouter ()
  {
    ;
  }

  void
  IrvineRouter::SetNocNode (Ptr<NocNode> nocNode)
  {
    NocRouter::SetNocNode (nocNode);

    m_internalLeftInputDevice->SetNode (m_nocNode);
    Ptr<Queue> leftInQueue = CreateObject<DropTailQueue> ();
    leftInQueue->SetAttribute ("MaxPackets", UintegerValue (UINT_MAX));
    m_internalLeftInputDevice->SetInQueue (leftInQueue);
    GetNocNode ()->AddDevice (m_internalLeftInputDevice);

    m_internalRightInputDevice->SetNode (m_nocNode);
    Ptr<Queue> rightInQueue = CreateObject<DropTailQueue> ();
    rightInQueue->SetAttribute ("MaxPackets", UintegerValue (UINT_MAX));
    m_internalRightInputDevice->SetInQueue (rightInQueue);
    GetNocNode ()->AddDevice (m_internalRightInputDevice);

    m_internalOutputDevice->SetNode (m_nocNode);
    Ptr<Queue> outQueue = CreateObject<DropTailQueue> ();
    outQueue->SetAttribute ("MaxPackets", UintegerValue (UINT_MAX));
    m_internalOutputDevice->SetInQueue (outQueue);
//    m_internalOutputDevice->SetReceiveCallback (MakeCallback (&Node::NonPromiscReceiveFromDevice, this));
    GetNocNode ()->AddDevice (m_internalOutputDevice);
  }

  Ptr<NocNetDevice>
  IrvineRouter::GetInjectionNetDevice (Ptr<Packet> packet, Ptr<NocNode> destination)
  {
    NS_LOG_FUNCTION_NOARGS();
    Ptr<NocNetDevice> netDevice = 0;

    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        NocHeader nocHeader;
        packet->PeekHeader (nocHeader);

        if (!nocHeader.IsEmpty ())
          {
            bool isEast = nocHeader.HasEastDirection ();
            if (!isEast)
              {
                netDevice = m_internalLeftInputDevice;
              }
            else
              {
                netDevice = m_internalRightInputDevice;
              }
            m_headPacketsInjectionNetDevice[packet->GetUid ()] = netDevice;
          }
      }
    else
      {
        uint32_t headUid = tag.GetPacketHeadUid ();
        netDevice = m_headPacketsInjectionNetDevice[headUid];
      }
    NS_ASSERT (netDevice != 0);
    NS_LOG_DEBUG ("Chosen injection net device is " << netDevice->GetAddress ());

    return netDevice;
  }

  std::vector<Ptr<NocNetDevice> >
  IrvineRouter::GetInjectionNetDevices ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    std::vector<Ptr<NocNetDevice> > devices;
    devices.insert (devices.begin (), m_internalRightInputDevice);
    devices.insert (devices.begin (), m_internalLeftInputDevice);

    return devices;
  }

  Ptr<NocNetDevice>
  IrvineRouter::GetReceiveNetDevice ()
  {
    NS_LOG_FUNCTION_NOARGS();
    Ptr<NocNetDevice> netDevice;

    netDevice = m_internalOutputDevice;
    NS_LOG_DEBUG ("The output net device of node is " << netDevice->GetAddress ());

    return netDevice;
  }

  /**
   * The device is added to the left or to the right router, based on its routing direction
   *
   * \see ns3::NocNetDevice#GetRoutingDirection()
   */
  uint32_t
  IrvineRouter::AddDevice (Ptr<NocNetDevice> device)
  {
    // we add all devices to m_devices as well
    uint32_t index = m_devices.size ();
    m_devices.push_back (device);
    NS_LOG_DEBUG ("Routing protocol is " << GetRoutingProtocol()->GetTypeId().GetName());
    // TODO this router knows to work only with 2D meshes (it is only aware of NORTH, SOUTH, EAST, WEST directions)
        switch (device->GetRoutingDirection ()) {
          case NocRoutingProtocol::NONE:
            NS_LOG_WARN("The net device " << device->GetAddress () << " has no routing direction!");
            break;
          case NocRoutingProtocol::FORWARD:
            if (device->GetRoutingDimension () == 1)
              {
                if (!m_north1DeviceAdded)
                  {
                    m_rightRouterInputDevices.push_back (device);
                    m_rightRouterOutputDevices.push_back (device);
                    m_north1DeviceAdded = true;
                  }
                else
                  {
                    NS_ASSERT(!m_north2DeviceAdded);
                    m_leftRouterInputDevices.push_back (device);
                    m_leftRouterOutputDevices.push_back (device);
                    m_north2DeviceAdded = true;
                  }
              }
            else
              {
                if (device->GetRoutingDimension () == 0)
                  {
                    NS_ASSERT(!m_eastDeviceAdded);
                    m_leftRouterInputDevices.push_back (device);
                    m_rightRouterOutputDevices.push_back (device);
                    m_eastDeviceAdded = true;
                  }
              }
            break;
          case NocRoutingProtocol::BACK:
            if (device->GetRoutingDimension () == 1)
              {
                if (!m_south1DeviceAdded)
                  {
                    m_rightRouterInputDevices.push_back (device);
                    m_rightRouterOutputDevices.push_back (device);
                    m_south1DeviceAdded = true;
                  }
                else
                  {
                    NS_ASSERT(!m_south2DeviceAdded);
                    m_leftRouterInputDevices.push_back (device);
                    m_leftRouterOutputDevices.push_back (device);
                    m_south2DeviceAdded = true;
                  }
              }
            else
              {
                if (device->GetRoutingDimension () == 0)
                  {
                    NS_ASSERT(!m_westDeviceAdded);
                    m_rightRouterInputDevices.push_back (device);
                    m_leftRouterOutputDevices.push_back (device);
                    m_westDeviceAdded = true;
                  }
              }
            break;
          default:
            break;
        }
    return index;
  }

  Ptr<NocNetDevice>
  IrvineRouter::GetInputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension)
  {
    NS_LOG_DEBUG ("Searching for an input net device for node " << GetNocNode ()->GetId ()
        << " and direction " << routingDirection);

    bool isRightIrvineRouter = isRightRouter (sender);
    bool isLeftIrvineRouter = isLeftRouter (sender);
    NS_ASSERT_MSG (isRightIrvineRouter || isLeftIrvineRouter, "The packet came through net device "
        << sender->GetAddress () << " This is not from right nor left router.");
    NS_ASSERT_MSG (!isRightIrvineRouter || !isLeftIrvineRouter, "The packet came through net device "
        << sender->GetAddress () << " This is from both right and left routers.");

    Ptr<NocNetDevice> netDevice = 0;
    // note that right and left routers correspond for north and south directions
    // but they don't for west and east (a west output from, a right router connects
    // to an east input, from a left router)
    if (isRightIrvineRouter)
      {
        NS_LOG_DEBUG ("The packet came through the right router");
        bool found = false;
        for (unsigned int i = 0; i < m_rightRouterInputDevices.size(); ++i)
          {
            Ptr<NocNetDevice> tmpNetDevice = m_rightRouterInputDevices[i]->GetObject<NocNetDevice> ();
            NS_LOG_DEBUG ("Right input " << tmpNetDevice->GetAddress ());
            if (tmpNetDevice->GetRoutingDirection () == routingDirection
                && tmpNetDevice->GetRoutingDimension () == routingDimension)
              {
                netDevice = tmpNetDevice;
                found = true;
                break;
              }
          }
        if (!found)
          {
            for (unsigned int i = 0; i < m_leftRouterInputDevices.size(); ++i)
              {
                Ptr<NocNetDevice> tmpNetDevice = m_leftRouterInputDevices[i]->GetObject<NocNetDevice> ();
                NS_LOG_DEBUG ("Left input " << tmpNetDevice->GetAddress ());
                if (tmpNetDevice->GetRoutingDirection () == routingDirection
                    && tmpNetDevice->GetRoutingDimension () == routingDimension)
                  {
                    netDevice = tmpNetDevice;
                    found = true;
                    break;
                  }
              }
          }
      }
    else
      {
        NS_LOG_DEBUG ("The packet came through the left router");
        bool found = false;
        for (unsigned int i = 0; i < m_leftRouterInputDevices.size(); ++i)
          {
            Ptr<NocNetDevice> tmpNetDevice = m_leftRouterInputDevices[i]->GetObject<NocNetDevice> ();
            NS_LOG_DEBUG ("Left input " << tmpNetDevice->GetAddress ());
            if (tmpNetDevice->GetRoutingDirection () == routingDirection
                && tmpNetDevice->GetRoutingDimension () == routingDimension)
              {
                netDevice = tmpNetDevice;
                found = true;
                break;
              }
          }
        if (!found)
          {
            for (unsigned int i = 0; i < m_rightRouterInputDevices.size(); ++i)
              {
                Ptr<NocNetDevice> tmpNetDevice = m_rightRouterInputDevices[i]->GetObject<NocNetDevice> ();
                NS_LOG_DEBUG ("Right input " << tmpNetDevice->GetAddress ());
                if (tmpNetDevice->GetRoutingDirection () == routingDirection
                    && tmpNetDevice->GetRoutingDimension () == routingDimension)
                  {
                    netDevice = tmpNetDevice;
                    found = true;
                    break;
                  }
              }
          }
      }
    if (netDevice)
      {
        NS_LOG_DEBUG ("Found net device " << netDevice->GetAddress ());
      }
    else
      {
        NS_LOG_DEBUG ("No net device found!");
      }
    return netDevice;
  }

  Ptr<NocNetDevice>
  IrvineRouter::GetOutputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension)
  {
    NS_LOG_DEBUG ("Searching for an output net device for node " << GetNocNode ()->GetId ()
        << " and direction " << routingDirection << " (sender net device is " << sender->GetAddress () << ")");

    bool isRightIrvineRouter = isRightRouter (sender);
    bool isLeftIrvineRouter = isLeftRouter (sender);
    NS_ASSERT_MSG (isRightIrvineRouter || isLeftIrvineRouter, "The packet came through net device "
        << sender->GetAddress () << " This is not from right nor left router.");
    NS_ASSERT_MSG (!isRightIrvineRouter || !isLeftIrvineRouter, "The packet came through net device "
        << sender->GetAddress () << " This is from both right and left routers.");

    Ptr<NocNetDevice> netDevice = 0;
    if (isRightIrvineRouter)
      {
        NS_LOG_DEBUG ("The packet came through the right router");
        for (unsigned int i = 0; i < m_rightRouterOutputDevices.size(); ++i)
          {
            Ptr<NocNetDevice> tmpNetDevice = m_rightRouterOutputDevices[i]->GetObject<NocNetDevice> ();
            NS_LOG_DEBUG ("Right output " << tmpNetDevice->GetAddress ());
            if (tmpNetDevice->GetRoutingDirection () == routingDirection
                && tmpNetDevice->GetRoutingDimension () == routingDimension)
              {
                netDevice = tmpNetDevice;
//                break;
              }
          }
      }
    else
      {
        NS_LOG_DEBUG ("The packet came through the left router");
        for (unsigned int i = 0; i < m_leftRouterOutputDevices.size(); ++i)
          {
            Ptr<NocNetDevice> tmpNetDevice = m_leftRouterOutputDevices[i]->GetObject<NocNetDevice> ();
            NS_LOG_DEBUG ("Left output " << tmpNetDevice->GetAddress ());
            if (tmpNetDevice->GetRoutingDirection () == routingDirection
                && tmpNetDevice->GetRoutingDimension () == routingDimension)
              {
                netDevice = tmpNetDevice;
//                break;
              }
          }
      }
    if (netDevice)
      {
        NS_LOG_DEBUG ("Found net device " << netDevice->GetAddress ());
      }
    else
      {
        NS_LOG_DEBUG ("No net device found!");
      }
    return netDevice;
  }

  std::vector<Ptr<NocNetDevice> >
  IrvineRouter::GetOutputNetDevices (Ptr<Packet> packet, Ptr<NocNetDevice> sender)
  {
    NS_LOG_FUNCTION (sender->GetAddress ());

    std::vector<Ptr<NocNetDevice> > outputDevices;

    bool isRightIrvineRouter = isRightRouter (sender);
    bool isLeftIrvineRouter = isLeftRouter (sender);
    NS_ASSERT_MSG (isRightIrvineRouter || isLeftIrvineRouter, "The packet came through net device "
        << sender->GetAddress () << " This is not from right nor left router.");
    NS_ASSERT_MSG (!isRightIrvineRouter || !isLeftIrvineRouter, "The packet came through net device "
        << sender->GetAddress () << " This is from both right and left routers.");


    NocHeader header;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        packet->PeekHeader (header);
      }
    NS_ASSERT (!header.IsEmpty ());

    if (isRightIrvineRouter)
      {
        NS_LOG_DEBUG ("The packet came through the right router");
        for (unsigned int i = 0; i < m_rightRouterOutputDevices.size(); ++i)
          {
            Ptr<NocNetDevice> tmpNetDevice = m_rightRouterOutputDevices[i]->GetObject<NocNetDevice> ();
            if (header.GetXOffset () == 0)
              {
                if (tmpNetDevice->GetRoutingDirection () != NocRoutingProtocol::FORWARD
                    && tmpNetDevice->GetRoutingDimension () != 0)
                  {
                    // we do not allow routing left once we are on the same column with the source
                    // because an Irvine router doesn't allow a packet to turn from West to East
                    outputDevices.insert (outputDevices.begin(), tmpNetDevice);
                  }
              }
            else
              {
                outputDevices.insert (outputDevices.begin(), tmpNetDevice);
              }
          }
      }
    else
      {
        NS_LOG_DEBUG ("The packet came through the left router");
        for (unsigned int i = 0; i < m_leftRouterOutputDevices.size(); ++i)
          {
            Ptr<NocNetDevice> tmpNetDevice = m_leftRouterOutputDevices[i]->GetObject<NocNetDevice> ();
            if (header.GetXOffset () == 0)
              {
                if (tmpNetDevice->GetRoutingDirection () != NocRoutingProtocol::FORWARD
                    && tmpNetDevice->GetRoutingDimension () != 0)
                  {
                    // we do not allow routing left once we are on the same column with the source
                    // because an Irvine router doesn't allow a packet to turn from East to West
                    outputDevices.insert (outputDevices.begin(), tmpNetDevice);
                  }
              }
            else
              {
                outputDevices.insert (outputDevices.begin(), tmpNetDevice);
              }
          }
      }

    return outputDevices;
  }

  bool
  IrvineRouter::isRightRouter (Ptr<NocNetDevice> sender)
  {
    bool isRightRouter = false;
    if (sender == m_internalRightInputDevice)
      {
        isRightRouter = true;
      }
    else
      {
        Ptr<IrvineRouter> router = sender->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetObject<IrvineRouter> ();
        for (unsigned int i = 0; i < router->m_rightRouterInputDevices.size(); ++i)
          {
            NS_LOG_DEBUG ("Comparing " << router->m_rightRouterInputDevices[i]->GetAddress ()
                << " with " << sender->GetAddress ());
            if (router->m_rightRouterInputDevices[i] == sender)
              {
                isRightRouter = true;
                break;
              }
          }
      }
    NS_LOG_DEBUG ("Comparison result: " << isRightRouter);
    return isRightRouter;
  }

  bool
  IrvineRouter::isLeftRouter (Ptr<NocNetDevice> sender)
  {
    bool isLeftRouter = false;
    if (sender == m_internalLeftInputDevice)
      {
        isLeftRouter = true;
      }
    else
      {
        Ptr<IrvineRouter> router = sender->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetObject<IrvineRouter> ();
        for (unsigned int i = 0; i < router->m_leftRouterInputDevices.size(); ++i)
          {
          NS_LOG_DEBUG ("Comparing " << router->m_leftRouterInputDevices[i]->GetAddress ()
              << " with " << sender->GetAddress ());
            if (router->m_leftRouterInputDevices[i] == sender)
              {
                isLeftRouter = true;
                break;
              }
          }
      }
    NS_LOG_DEBUG ("Comparison result: " << isLeftRouter);
    return isLeftRouter;
  }

  double
  IrvineRouter::GetInChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    NS_ASSERT (sourceDevice != 0);

    double occupancy = 0;
    uint32_t packets = 0;
    uint64_t sizes = 0;

    if (isLeftRouter (sourceDevice))
      {
        for (unsigned int i = 0; i < m_leftRouterInputDevices.size (); ++i)
          {
            Ptr<NocNetDevice> device = m_leftRouterInputDevices[i];
            packets += device->GetInQueueNPacktes ();
            sizes += device->GetInQueueSize ();
          }
      }
    else
      {
        if (isRightRouter (sourceDevice))
          {
            for (unsigned int i = 0; i < m_rightRouterInputDevices.size (); ++i)
              {
                Ptr<NocNetDevice> device = m_rightRouterInputDevices[i];
                packets += device->GetInQueueNPacktes ();
                sizes += device->GetInQueueSize ();
              }
          }
        else
          {
            NS_LOG_ERROR ("The net device " << sourceDevice->GetAddress ()
                << " is not an input net device of this router!");
          }
      }
    if (sizes != 0 && packets != 0)
      {
        occupancy = packets * 1.0 / sizes;
      }
    NS_LOG_LOGIC ("In channels occupancy is " << occupancy);

    return occupancy;
  }

  double
  IrvineRouter::GetOutChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    NS_ASSERT (sourceDevice != 0);

    double occupancy = 0;
    uint32_t packets = 0;
    uint64_t sizes = 0;

    if (isLeftRouter (sourceDevice))
      {
        for (unsigned int i = 0; i < m_leftRouterOutputDevices.size (); ++i)
          {
            Ptr<NocNetDevice> device = m_leftRouterOutputDevices[i];
            packets += device->GetOutQueueNPacktes ();
            sizes += device->GetOutQueueSize ();
          }
      }
    else
      {
        if (isRightRouter (sourceDevice))
          {
            for (unsigned int i = 0; i < m_rightRouterOutputDevices.size (); ++i)
              {
                Ptr<NocNetDevice> device = m_rightRouterOutputDevices[i];
                packets += device->GetOutQueueNPacktes ();
                sizes += device->GetOutQueueSize ();
              }
          }
        else
          {
            NS_LOG_ERROR ("The net device " << sourceDevice->GetAddress ()
                << " is not an output net device of this router!");
          }
      }
    if (sizes != 0 && packets != 0)
      {
        occupancy = packets * 1.0 / sizes;
      }
    NS_LOG_LOGIC ("Out channels occupancy is " << occupancy);

    return occupancy;
  }

  uint32_t
  IrvineRouter::GetNumberOfInputPorts ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    return 8; // injection left + injection right + N1 + N2 + E + S1 + S2 + W
  }

  uint32_t
  IrvineRouter::GetNumberOfOutputPorts ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    return 7; // ejection + N1 + N2 + E + S1 + S2 + W
  }

  /**
   * \return how many virtual channels the router has
   */
  uint32_t
  IrvineRouter::GetNumberOfVirtualChannels ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    // TODO change this if virtual channels are added
    return 0;
  }

} // namespace ns3
