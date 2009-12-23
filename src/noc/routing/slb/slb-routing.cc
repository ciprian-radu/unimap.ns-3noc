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

    NS_LOG_DEBUG ("The following output net devices can be used for routing the packet "
        << packet << " (which came from " << source->GetAddress () << ")");
    for (unsigned int i = 0; i < validDevices.size (); ++i) {
      NS_LOG_DEBUG (validDevices[i]->GetAddress ());
    }
    NS_LOG_DEBUG ("");

    return validDevices;
  }

  Ptr<NocNetDevice>
  SlbRouting::DoSelectionFunction (std::vector<Ptr<NocNetDevice> > devices,
      const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    Ptr<NocNetDevice> device;

    // TODO a random selection is performed (in NoCSim) among devices with with the same value
    int bestDeviceValue = -1;
    for (unsigned int i = 0; i < devices.size (); ++i)
      {
        int deviceValue = Evaluate (devices[i], packet);
        if (deviceValue > bestDeviceValue)
          {
            device = devices[i];
            bestDeviceValue = deviceValue;
          }
      }

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

    if (header.GetXDistance() > 0 && device->GetRoutingDirection () == NocRoutingProtocol::EAST)
      {
        isProgressive = true;
      }
    else
      {
        if (header.GetXDistance() < 0 && device->GetRoutingDirection () == NocRoutingProtocol::WEST)
          {
            isProgressive = true;
          }
        else
          {
            if (header.GetYDistance() < 0 && device->GetRoutingDirection () == NocRoutingProtocol::NORTH)
              {
                isProgressive = true;
              }
            else
              {
                if (header.GetYDistance() > 0 && device->GetRoutingDirection () == NocRoutingProtocol::SOUTH)
                  {
                    isProgressive = true;
                  }
              }
          }
      }

    return isProgressive;
  }

} // namespace ns3
