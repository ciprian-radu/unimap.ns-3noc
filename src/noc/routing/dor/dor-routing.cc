/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *                                                              Lucian Blaga University of Sibiu, Romania
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
 * Authors: Andreea Gancea <andreea.gancea@ulbsibiu.ro>
 *          Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 *                      http://webspace.ulbsibiu.ro/ciprian.radu/
 */

#include "dor-routing.h"
#include "ns3/log.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/boolean.h"
#include "ns3/noc-value.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/pointer.h"
#include "ns3/noc-registry.h"

NS_LOG_COMPONENT_DEFINE ("DorRouting");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (DorRouting);

  TypeId
  DorRouting::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::DorRouting")
             .SetParent<NocRoutingProtocol> ()
             .AddConstructor<DorRouting> ()
             .AddAttribute ("RoutingOrder",
                  "In what order the dimensions are routed (a permutation of the numbers from 0 to n, where n is the dimensions number)",
                  ObjectVectorValue(),
                  MakeObjectVectorAccessor (&DorRouting::m_routingOrder),
                  MakeObjectVectorChecker<NocValue> ())
                 ;
    return tid;
  }

   // we could easily name the protocol "DOR", but using __FILE__ should be more useful for debugging
  DorRouting::DorRouting () : NocRoutingProtocol (__FILE__)
   {
    NS_LOG_FUNCTION_NOARGS ();
    IntegerValue integerValue;
    NocRegistry::GetInstance ()->GetAttribute ("NoCDimensions", integerValue);
    uint32_t topologyDimension = integerValue.Get ();

    Ptr<NocValue> value = CreateObject<NocValue> ();
    for (unsigned int i = 0; i < topologyDimension; i++)
      {
        value = CreateObject<NocValue> ();
        value->SetValue (i);
        m_routingOrder.insert (m_routingOrder.end(),value);
      }
   }

  DorRouting::DorRouting (std::vector<Ptr<NocValue> > routingOrder) : NocRoutingProtocol (__FILE__)
  {
     NS_LOG_FUNCTION ("(with routing order)");
     m_routingOrder = routingOrder;
  }

  DorRouting::~DorRouting ()
  {
    ;
  }

  void
  DorRouting::SetRoute (std::vector<Ptr<NocValue> > route)
  {
    m_routingOrder=route;
  }

  std::vector<Ptr<NocValue> >
  DorRouting::GetRoute () const
  {
    return m_routingOrder;
  }

  Ptr<Route>
  DorRouting::RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION_NOARGS ();
    std::stringstream ss;
    packet->Print (ss);
    NS_LOG_DEBUG ("source node = " << source->GetNode ()->GetId () << ", destination node = " << destination->GetId ()
        << ", packet " << ss.str ());
    ss.str ("");

    NocHeader nocHeader;
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    NS_ASSERT (NocPacket::HEAD == tag.GetPacketType ());
    packet->RemoveHeader (nocHeader);

    uint dimension=m_routingOrder.size();

    std::vector<bool> isForward(dimension);
    std::vector<uint8_t> offset(dimension);

    for (unsigned int i = 0; i < m_routingOrder.size(); i++)
    {
        isForward.at(i) = nocHeader.HasForwardDirection(i);
        offset.at(i) = nocHeader.GetOffset().at(i);
        NS_LOG_DEBUG ("dimension " << (i+1) << " Offset " << (uint) offset.at(i) << " direction " << (isForward.at(i) ? "forward" : "back"));
    }

    std::vector <RoutingDirection> direction(m_routingOrder.size(), NocRoutingProtocol::NONE);

    dimension = 0;

    for (unsigned int i = 0; i < m_routingOrder.size (); i++)
      {
        dimension = 0;
        while (m_routingOrder.at (i)->GetValue () != dimension)
          {
            dimension++;
          }

        if (offset.at (dimension) != 0)
          {
            if (isForward.at (dimension))
              {
                NS_ASSERT_MSG (offset.at (dimension) - 1 >= 0, "A packet going Forward will have the offset < 0");
                direction.at (dimension) = NocRoutingProtocol::FORWARD;
              }
            else
              {
                NS_ASSERT_MSG (offset.at (dimension) - 1 >= 0, "A packet going Back will have the offset < 0");
                direction.at (dimension) = NocRoutingProtocol::BACK;
              }
            offset.at (dimension)--;
            nocHeader.SetOffset(offset);
            break;
          }
      }

    for (unsigned int i = 0; i < m_routingOrder.size (); i++)
      {
        NS_LOG_DEBUG ("dimension " << (i+1) << " new offset " << (int) nocHeader.GetOffset().at(i));
      }

    packet->AddHeader (nocHeader);

    packet->Print (ss);
    NS_LOG_DEBUG ("source node = " << source->GetNode ()->GetId () << ", destination node = " << destination->GetId ()
        << ", packet " << ss.str ());
    ss.str ("");

    Ptr<Route> route = 0;

    std::vector <bool> routeDimension(m_routingOrder.size(), true);

    for (unsigned int i = 0; i < m_routingOrder.size (); i++)
      {
        switch (direction.at (i))
          {
            case NocRoutingProtocol::FORWARD:
              m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetOutputNetDevice (source, FORWARD, i);
              NS_ASSERT (m_sourceNetDevice != 0);
              m_destinationNetDevice = destination->GetRouter ()->GetInputNetDevice (m_sourceNetDevice, BACK, i);
              NS_ASSERT (m_destinationNetDevice != 0);
              route = CreateObject<Route> (packet, m_sourceNetDevice, m_destinationNetDevice);
              break;
            case NocRoutingProtocol::BACK:
              m_sourceNetDevice = source->GetNode ()->GetObject<NocNode> ()->GetRouter ()->GetOutputNetDevice (source, BACK, i);
              NS_ASSERT (m_sourceNetDevice != 0);
              m_destinationNetDevice = destination->GetRouter ()->GetInputNetDevice (m_sourceNetDevice, FORWARD, i);
              NS_ASSERT (m_destinationNetDevice != 0);
              route = CreateObject<Route> (packet, m_sourceNetDevice, m_destinationNetDevice);
              break;
            case NocRoutingProtocol::NONE:
            default:
              routeDimension.at(i) = false;
              break;
          }
      }

    bool r = false;
    for (unsigned int i = 0; i < m_routingOrder.size (); i++)
      {
        r = r || routeDimension.at (i);
        if (r)
          {
            break;
          }
      }

    if (!r)
      {
        NS_LOG_WARN ("No routing needs to be performed!");
      }

    return route;
  }

} // namespace ns3
