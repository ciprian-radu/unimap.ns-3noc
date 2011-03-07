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

#include "4-way-router.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "limits.h"
#include "ns3/queue.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE ("FourWayRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (FourWayRouter);

  TypeId
  FourWayRouter::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::FourWayRouter")
        .SetParent<NocRouter> ()
        .AddConstructor<FourWayRouter> ();
    return tid;
  }

  // we could easily name the router "four way router", but using __FILE__ should be more useful for debugging
  FourWayRouter::FourWayRouter () : NocRouter (__FILE__)
  {
    Init ();
  }

  FourWayRouter::FourWayRouter (std::string name) : NocRouter (name)
  {
    Init ();
  }

  void
  FourWayRouter::Init ()
  {
    m_internalInputDevice = CreateObject<NocNetDevice> ();
    Ptr<Queue> leftInQueue = CreateObject<DropTailQueue> ();
    leftInQueue->SetAttribute ("MaxPackets", UintegerValue (UINT_MAX));
    m_internalInputDevice->SetInQueue (leftInQueue);
    m_internalInputDevice->SetAddress (Mac48Address::Allocate ());

    m_internalOutputDevice = CreateObject<NocNetDevice> ();
    Ptr<Queue> outQueue = CreateObject<DropTailQueue> ();
    outQueue->SetAttribute ("MaxPackets", UintegerValue (UINT_MAX));
    m_internalOutputDevice->SetInQueue (outQueue);
    m_internalOutputDevice->SetAddress (Mac48Address::Allocate ());
  }

  FourWayRouter::~FourWayRouter ()
  {
    ;
  }

  void
  FourWayRouter::SetNocNode (Ptr<NocNode> nocNode)
  {
    NocRouter::SetNocNode (nocNode);

    m_internalInputDevice->SetNode (m_nocNode);
    GetNocNode ()->AddDevice (m_internalInputDevice);

    m_internalOutputDevice->SetNode (m_nocNode);
    //    m_internalOutputDevice->SetReceiveCallback (MakeCallback (&Node::NonPromiscReceiveFromDevice, this));
    GetNocNode ()->AddDevice (m_internalOutputDevice);
  }

  Ptr<NocNetDevice>
  FourWayRouter::GetInjectionNetDevice (Ptr<Packet> packet, Ptr<NocNode> destination)
  {
    NS_LOG_FUNCTION_NOARGS();
    Ptr<NocNetDevice> netDevice;

    netDevice = m_internalInputDevice;
    NS_LOG_DEBUG ("The input net device of node " << destination->GetId ()
        << " is " << netDevice->GetAddress ());

    return netDevice;
  }

  std::vector<Ptr<NocNetDevice> >
  FourWayRouter::GetInjectionNetDevices ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    std::vector<Ptr<NocNetDevice> > devices;
    devices.insert (devices.begin (), m_internalInputDevice);

    return devices;
  }

  Ptr<NocNetDevice>
  FourWayRouter::GetReceiveNetDevice ()
  {
    NS_LOG_FUNCTION_NOARGS();
    Ptr<NocNetDevice> netDevice;

    netDevice = m_internalOutputDevice;
    NS_LOG_DEBUG ("The output net device of is " << netDevice->GetAddress ());

    return netDevice;
  }

  Ptr<NocNetDevice>
  FourWayRouter::GetInputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension)
  {
    NS_LOG_DEBUG ("Searching for a net device for node " << GetNocNode ()->GetId () << " and direction " << routingDirection);
    Ptr<NocNetDevice> netDevice = 0;
    for (unsigned int i = 0; i < GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> tmpNetDevice = GetDevice (i)->GetObject<NocNetDevice> ();
        if (tmpNetDevice->GetRoutingDirection () == routingDirection
            && tmpNetDevice->GetRoutingDimension () == routingDimension)
          {
            netDevice = tmpNetDevice;
            break;
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
  FourWayRouter::GetOutputNetDevice (Ptr<NocNetDevice> sender, const int routingDirection, const int routingDimension)
  {
    return GetInputNetDevice (sender, routingDirection, routingDimension);
  }

  std::vector<Ptr<NocNetDevice> >
  FourWayRouter::GetOutputNetDevices (Ptr<Packet> packet, Ptr<NocNetDevice> sender)
  {
    NS_LOG_FUNCTION (sender->GetAddress ());

    std::vector<Ptr<NocNetDevice> > outputDevices;

    for (unsigned int i = 0; i < GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> tmpNetDevice = GetDevice (i)->GetObject<NocNetDevice> ();
        outputDevices.insert (outputDevices.begin(), tmpNetDevice);
      }

    return outputDevices;
  }

  uint32_t
  FourWayRouter::GetNumberOfInputPorts ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    return 5; // injection + N + E + S + W
  }

  uint32_t
  FourWayRouter::GetNumberOfOutputPorts ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    return 5; // ejection + N + E + S + W
  }

  /**
   * \return how many virtual channels the router has
   */
  uint32_t
  FourWayRouter::GetNumberOfVirtualChannels ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    // TODO change this if virtual channels are added
    return 0;
  }

} // namespace ns3
