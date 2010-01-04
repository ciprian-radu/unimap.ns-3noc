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
#include "ns3/noc-channel.h"
#include "ns3/random-variable.h"
#include <vector>

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
        ;
    return tid;
  }

  // we could easily name the protocol "Static Load Bound", but using __FILE__ should be more useful for debugging
  SlbRouting::SlbRouting () : NocRoutingProtocol (__FILE__)
  {
    progressiveWeight = 2;
    remainingWeight = -1;
    loadWeight = -4;
  }

  SlbRouting::~SlbRouting ()
  {
    ;
  }

  bool
  SlbRouting::RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination,
      Ptr<Packet> packet, RouteReplyCallback routeReply)
  {
    NS_LOG_FUNCTION_NOARGS ();

    std::vector<Ptr<NocNetDevice> > devices = DoRoutingFunction (source, destination, packet);
    Ptr<NocNetDevice> selectedDevice = DoSelectionFunction(devices, source, destination, packet);
    UpdateHeader (packet, selectedDevice);

    m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->
        GetOutputNetDevice(source, selectedDevice->GetRoutingDirection ());
    NS_ASSERT(m_sourceNetDevice != 0);
    m_destinationNetDevice = destination->GetRouter ()->
        GetInputNetDevice(m_sourceNetDevice,
            NocRoutingProtocol::GetOpositeDirection2DMesh (selectedDevice->GetRoutingDirection ()));

    // TODO
    if (m_destinationNetDevice == 0)
      {
        m_destinationNetDevice = destination->GetRouter ()->
            GetInputNetDevice(m_sourceNetDevice,
                NocRoutingProtocol::GetOpositeDirection2DMesh (NocRoutingProtocol::NORTH));
        if (m_destinationNetDevice == 0)
          {
            m_destinationNetDevice = destination->GetRouter ()->
                GetInputNetDevice(m_sourceNetDevice,
                    NocRoutingProtocol::GetOpositeDirection2DMesh (NocRoutingProtocol::EAST));
            if (m_destinationNetDevice == 0)
              {
                m_destinationNetDevice = destination->GetRouter ()->
                    GetInputNetDevice(m_sourceNetDevice,
                        NocRoutingProtocol::GetOpositeDirection2DMesh (NocRoutingProtocol::SOUTH));
                if (m_destinationNetDevice == 0)
                  {
                      m_destinationNetDevice = destination->GetRouter ()->
                          GetInputNetDevice(m_sourceNetDevice,
                              NocRoutingProtocol::GetOpositeDirection2DMesh (NocRoutingProtocol::WEST));
                  }
              }
          }
      }

    NS_ASSERT(m_destinationNetDevice != 0);
    routeReply (packet, m_sourceNetDevice, m_destinationNetDevice);

    return true;
  }

  std::vector<Ptr<NocNetDevice> >
  SlbRouting::DoRoutingFunction (const Ptr<NocNetDevice> source,
      const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    std::vector<Ptr<NocNetDevice> > validDevices;

    std::vector<Ptr<NocNetDevice> > devices = source->GetNode ()->GetObject<NocNode> ()->
        GetRouter ()->GetOutputNetDevices (source);

    for (unsigned int i = 0; i < devices.size (); ++i) {
      if (devices[i]->GetRoutingDirection() != source->GetRoutingDirection ())
        {
          validDevices.insert (validDevices.begin(), devices[i]);
        }
    }

    if (validDevices.size() == 0)
      {
        NS_LOG_DEBUG ("No output net devices can be used for routing the packet "
            << packet << " (which came from " << source->GetAddress () << ")");
      }
    else
      {
        NS_LOG_DEBUG ("The following output net devices can be used for routing the packet "
            << packet << " (which came from " << source->GetAddress () << ")");
        for (unsigned int i = 0; i < validDevices.size (); ++i) {
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
    device = selectedDevices[randomVariable.GetValue (0, selectedDevices.size ())];

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
        value += progressiveWeight;
      }

    // loaded directions are bad
    // FIXME

    // currently busy directions are also bad
    Ptr<NocChannel> channel = device->GetChannel()->GetObject<NocChannel> ();
    if (channel->IsBusy ())
      {
        value += remainingWeight;
      }

    return value;
  }

  bool
  SlbRouting::IsProgressiveDirection (Ptr<Packet> packet, Ptr<NocNetDevice> device)
  {
    bool isProgressive = false;

    NocHeader header;
    packet->PeekHeader (header);
    NS_ASSERT (!header.IsEmpty());

    NS_LOG_DEBUG ("Head packet X distance is " << (int) header.GetXDistance());
    NS_LOG_DEBUG ("Head packet Y distance is " << (int) header.GetYDistance());

    if (((header.GetXDistance() & 8) == 0) && header.GetXDistance() > 0
        && device->GetRoutingDirection () == NocRoutingProtocol::EAST)
      {
        isProgressive = true;
      }
    else
      {
        if (((header.GetXDistance() & 8) == 8) && header.GetXDistance() > 0
            && device->GetRoutingDirection () == NocRoutingProtocol::WEST)
          {
            isProgressive = true;
          }
        else
          {
            if (((header.GetYDistance() & 8) == 8)  && header.GetYDistance() > 0
                && device->GetRoutingDirection () == NocRoutingProtocol::NORTH)
              {
                isProgressive = true;
              }
            else
              {
                if (((header.GetYDistance() & 8) == 0)  && header.GetYDistance() > 0
                    && device->GetRoutingDirection () == NocRoutingProtocol::SOUTH)
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
  SlbRouting::UpdateHeader (Ptr<Packet> packet, Ptr<NocNetDevice> device)
  {
    NocHeader nocHeader;
    packet->RemoveHeader (nocHeader);
    NS_ASSERT (!nocHeader.IsEmpty ());

    uint8_t xDistance = nocHeader.GetXDistance ();
//    bool isEast = (xDistance & 0x08) != 0x08;
    int xOffset = xDistance & 0x07;

    uint8_t yDistance = nocHeader.GetYDistance ();
//    bool isSouth = (yDistance & 0x08) != 0x08;
    int yOffset = yDistance & 0x07;

    switch (device->GetRoutingDirection ()) {
      case NORTH:
        yOffset--;
        nocHeader.SetYDistance (0x08 | yOffset);
        break;
      case EAST:
        xOffset--;
        nocHeader.SetXDistance (0x07 & xOffset);
        break;
      case SOUTH:
        yOffset--;
        nocHeader.SetYDistance (0x07 & yOffset);
        break;
      case WEST:
        xOffset--;
        nocHeader.SetXDistance (0x08 | xOffset);
        break;
      case NONE:
      default:
        NS_LOG_ERROR ("Unknown routing direction");
        break;
    }

    packet->AddHeader (nocHeader);
  }

} // namespace ns3
