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

#include "slb-routing.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/noc-channel.h"
#include "ns3/random-variable.h"
#include "ns3/integer.h"
#include <vector>
#include <limits.h>

NS_LOG_COMPONENT_DEFINE ("SlbRouting");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (SlbRouting);

  TypeId
  SlbRouting::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::SlbRouting")
        .SetParent<NocRoutingProtocol> ()
        .AddConstructor<SlbRouting> ()
        .AddAttribute (
            "ProgressiveWeight",
            "the weight used for a progressive direction (i.e. that leads to destination)",
            IntegerValue (2),
            MakeIntegerAccessor (&SlbRouting::m_progressiveWeight),
            MakeIntegerChecker<int> ())
        .AddAttribute (
            "RemainingWeight",
            "the weight used for a direction which is to loaded",
            IntegerValue (-1),
            MakeIntegerAccessor (&SlbRouting::m_remainingWeight),
            MakeIntegerChecker<int> ())
        .AddAttribute (
            "LoadWeight",
            "the weight used for a direction where the channel is busy",
            IntegerValue (-4),
            MakeIntegerAccessor (&SlbRouting::m_loadWeight),
            MakeIntegerChecker<int> ())
        .AddAttribute (
            "LoadThreshold",
            "The threshold for the load of a router (per routing direction)",
            IntegerValue (50),
            MakeIntegerAccessor (&SlbRouting::SetLoadThreshold, &SlbRouting::GetLoadThreshold),
            MakeIntegerChecker<int> (0, INT_MAX))
        ;
    return tid;
  }

  // we could easily name the protocol "Static Load Bound", but using __FILE__ should be more useful for debugging
  SlbRouting::SlbRouting () : NocRoutingProtocol (__FILE__)
  {
    ;
  }

  SlbRouting::~SlbRouting ()
  {
    ;
  }

  int
  SlbRouting::GetLoadThreshold () const
  {
    NS_LOG_FUNCTION_NOARGS ();
    return m_loadThreshold;
  }

  void
  SlbRouting::SetLoadThreshold (int loadThreshold)
  {
    NS_LOG_FUNCTION (loadThreshold);
    m_loadThreshold = loadThreshold;
  }

  Ptr<Route>
  SlbRouting::RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();

    NocHeader nocHeader;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
        {
          packet->PeekHeader (nocHeader);
        }
    NS_ASSERT (!nocHeader.IsEmpty ());

    std::vector<Ptr<NocNetDevice> > devices = DoRoutingFunction (source, destination, packet);
    Ptr<NocNetDevice> selectedDevice = DoSelectionFunction(devices, source, destination, packet);
    UpdateHeader (packet, selectedDevice, source);

    m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->
        GetOutputNetDevice(source, selectedDevice->GetRoutingDirection (), selectedDevice->GetRoutingDimension ());
    NS_ASSERT(m_sourceNetDevice != 0);
    m_destinationNetDevice = destination->GetRouter ()-> GetInputNetDevice (m_sourceNetDevice,
        NocRoutingProtocol::GetOpositeRoutingDirection (selectedDevice->GetRoutingDirection ()),
        selectedDevice->GetRoutingDimension ());

    // ensure that we find the opposite net device at the destination node
    if (m_destinationNetDevice == 0)
      {
        m_destinationNetDevice = destination->GetRouter ()-> GetInputNetDevice (m_sourceNetDevice,
            NocRoutingProtocol::GetOpositeRoutingDirection (NocRoutingProtocol::FORWARD), 1);
        if (m_destinationNetDevice == 0)
          {
            m_destinationNetDevice = destination->GetRouter ()-> GetInputNetDevice (m_sourceNetDevice,
                NocRoutingProtocol::GetOpositeRoutingDirection (NocRoutingProtocol::FORWARD), 0);
            if (m_destinationNetDevice == 0)
              {
                m_destinationNetDevice = destination->GetRouter ()-> GetInputNetDevice (m_sourceNetDevice,
                    NocRoutingProtocol::GetOpositeRoutingDirection (NocRoutingProtocol::BACK), 1);
                if (m_destinationNetDevice == 0)
                  {
                    m_destinationNetDevice = destination->GetRouter ()-> GetInputNetDevice (m_sourceNetDevice,
                        NocRoutingProtocol::GetOpositeRoutingDirection (NocRoutingProtocol::BACK), 0);
                  }
              }
          }
      }

    NS_ASSERT(m_destinationNetDevice != 0);
    NS_LOG_DEBUG ("Found source net device " << m_sourceNetDevice->GetAddress ()
        << " (packet UID " << packet->GetUid () << ")");
    NS_LOG_DEBUG ("Found destination net device " << m_destinationNetDevice->GetAddress ()
        << " (packet UID " << packet->GetUid () << ")");

    Ptr<Route> route = CreateObject<Route> (packet, m_sourceNetDevice, m_destinationNetDevice);

    return route;
  }

  std::vector<Ptr<NocNetDevice> >
  SlbRouting::DoRoutingFunction (const Ptr<NocNetDevice> source,
      const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    std::vector<Ptr<NocNetDevice> > validDevices;

    std::vector<Ptr<NocNetDevice> > devices = source->GetNode ()->GetObject<NocNode> ()->
        GetRouter ()->GetOutputNetDevices (packet, source);

    for (unsigned int i = 0; i < devices.size (); ++i)
      {
        if (devices[i]->GetRoutingDirection() != source->GetRoutingDirection ())
          {
            validDevices.insert (validDevices.begin(), devices[i]);
          }
      }

    if (validDevices.size() == 0)
      {
        NS_LOG_DEBUG ("No output net devices can be used for routing the packet "
            << *packet << " (which came from " << source->GetAddress () << ")");
      }
    else
      {
        NS_LOG_DEBUG ("The following output net devices can be used for routing the packet "
            << packet << " (which came from " << source->GetAddress () << ")");
        for (unsigned int i = 0; i < validDevices.size (); ++i)
          {
            NS_LOG_DEBUG (validDevices[i]->GetAddress ());
          }
      }

    return validDevices;
  }

  Ptr<NocNetDevice>
  SlbRouting::DoSelectionFunction (std::vector<Ptr<NocNetDevice> > devices,
      const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    Ptr<NocNetDevice> device;
    std::vector<Ptr<NocNetDevice> > selectedDevices;

    int bestDeviceValue = -1;
    for (unsigned int i = 0; i < devices.size (); ++i)
      {
        int deviceValue = Evaluate (devices[i], packet);
        if (deviceValue == bestDeviceValue)
          {
            selectedDevices.insert (selectedDevices.begin (), devices[i]);
          }
        if (deviceValue > bestDeviceValue)
          {
            selectedDevices.erase (selectedDevices.begin (), selectedDevices.end ());
            selectedDevices.insert (selectedDevices.begin (), devices[i]);
            bestDeviceValue = deviceValue;
          }
      }

    // when we have more than one net devices which have the best value,
    // the selected net device is randomly picked
    for (unsigned int i = 0; i < selectedDevices.size(); ++i)
      {
        NS_LOG_DEBUG ("Net device " << selectedDevices[i]->GetAddress () << " is among the selected devices");
      }

    UniformVariable randomVariable;
    device = selectedDevices[(int)randomVariable.GetValue (0, selectedDevices.size ())];

    NS_LOG_DEBUG ("The net device " << device->GetAddress () << " was selected for routing");

    return device;
  }

  int
  SlbRouting::Evaluate (Ptr<NocNetDevice> device, Ptr<Packet> packet)
  {
    int value = 0;

    // progressive directions are good
    if (IsProgressiveDirection (packet, device))
      {
        value += m_progressiveWeight;
        NS_LOG_DEBUG ("Net device " << device->GetAddress ()
            << " is progressive (adding " << m_progressiveWeight << ")");
      }

    // loaded directions are bad
    Ptr<NocRouter> router = device->GetNode ()->GetObject<NocNode> ()->GetRouter ();
    if (router->GetNeighborLoad (device) > m_loadThreshold)
      {
        value += m_loadWeight;
        NS_LOG_DEBUG ("Net device " << device->GetAddress ()
            << " is leads to a loaded router (adding " << m_loadWeight << ")");
      }

    // currently busy directions are also bad
    Ptr<NocChannel> channel = device->GetChannel()->GetObject<NocChannel> ();
    if (channel->IsBusy (device))
      {
        value += m_remainingWeight;
        NS_LOG_DEBUG ("Net device " << device->GetAddress ()
            << " can be reached through a busy channel (adding " << m_remainingWeight << ")");
      }

    NS_LOG_LOGIC ("Net device " << device->GetAddress () << " was evaluated to " << value);

    return value;
  }

  bool
  SlbRouting::IsProgressiveDirection (Ptr<Packet> packet, Ptr<NocNetDevice> device)
  {
    bool isProgressive = false;

    NocHeader header;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
        {
          packet->PeekHeader (header);
        }
    NS_ASSERT (!header.IsEmpty());

    NS_LOG_DEBUG ("xOffset " << header.GetXOffset () << " direction "
        << (header.HasEastDirection () ? "east" : "west"));
    NS_LOG_DEBUG ("yOffset " << header.GetYOffset () << " direction "
        << (header.HasSouthDirection () ? "south" : "north"));

    if (((header.HasEastDirection ()) && (header.GetXOffset ()) > 0)
        && device->GetRoutingDirection () == NocRoutingProtocol::FORWARD
        && device->GetRoutingDimension () == 0)
      {
        isProgressive = true;
      }
    else
      {
        if (((header.HasWestDirection ()) && (header.GetXOffset ()) > 0)
            && device->GetRoutingDirection () == NocRoutingProtocol::BACK
            && device->GetRoutingDimension () == 0)
          {
            isProgressive = true;
          }
        else
          {
            if (((header.HasNorthDirection ())  && (header.GetYOffset ()) > 0)
                && device->GetRoutingDirection () == NocRoutingProtocol::FORWARD
                && device->GetRoutingDimension () == 1)
              {
                isProgressive = true;
              }
            else
              {
                if (((header.HasSouthDirection ())  && (header.GetYOffset ()) > 0)
                    && device->GetRoutingDirection () == NocRoutingProtocol::BACK
                    && device->GetRoutingDimension () == 1)
                  {
                    isProgressive = true;
                  }
              }
          }
      }

    if (isProgressive)
      {
        NS_LOG_DEBUG ("Net device " << device->GetAddress ()
            << " provides a progressive direction for packet " << packet);
      }
    else
      {
        NS_LOG_DEBUG ("Net device " << device->GetAddress ()
            << " does not provide a progressive direction for packet " << packet);
      }

    return isProgressive;
  }

  void
  SlbRouting::UpdateHeader (Ptr<Packet> packet, Ptr<NocNetDevice> device, const Ptr<NocNetDevice> source)
  {
    NS_LOG_FUNCTION (packet << device->GetAddress () << source->GetAddress ());
    NocHeader nocHeader;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        packet->RemoveHeader (nocHeader);
      }
    NS_ASSERT (!nocHeader.IsEmpty ());

    int xOffset = nocHeader.GetXOffset ();
    int yOffset = nocHeader.GetYOffset ();

    switch (device->GetRoutingDirection ()) {
      case FORWARD:
      if (device->GetRoutingDimension () == 0)
        {
          xOffset--;
          nocHeader.SetXOffset (xOffset);
        }
      else
        {
          if (device->GetRoutingDimension () == 1)
            {
              yOffset--;
              nocHeader.SetYOffset (yOffset);
            }
        }
        break;
      case BACK:
      if (device->GetRoutingDimension () == 0)
        {
          xOffset--;
          nocHeader.SetXOffset (xOffset);
        }
      else
        {
          if (device->GetRoutingDimension () == 1)
            {
              yOffset--;
              nocHeader.SetYOffset (yOffset);
            }
        }
      break;
      case NONE:
      default:
        NS_LOG_ERROR ("Unknown routing direction");
        break;
    }
    // we need the packet to have its header for calling GetLoadForDirection (...)
    packet->AddHeader (nocHeader);

    Ptr<NocRouter> router = source->GetNode ()->GetObject<NocNode> ()->GetRouter ();
    Ptr<LoadRouterComponent> loadComponent = router->GetLoadRouterComponent ();
    if (loadComponent != 0)
      {
        int load = loadComponent->GetLoadForDirection (packet, source, device);
        NS_ASSERT_MSG (load >= 0 && load <= 100, "The load of a router must be a percentage number ("
            << load << " is not)");
        NS_LOG_DEBUG ("Packet " << packet << " will propagate load " << load);
        tag.SetLoad (load);
      }
    packet->RemoveHeader (nocHeader);
    packet->AddHeader (nocHeader);
  }

} // namespace ns3
