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

#include "noc-registry.h"
#include "ns3/log.h"
#include "ns3/integer.h"
#include "ns3/object-factory.h"

NS_LOG_COMPONENT_DEFINE ("NocRegistry");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocRegistry);

  NocRegistry::NocRegistry ()
  {
    ;
  }

  Ptr<NocRegistry>
  NocRegistry::GetInstance ()
  {
    ObjectFactory factory;
    factory.SetTypeId (GetTypeId ());

//    static Ptr<NocRegistry> instance = CreateObject<NocRegistry> ();
    static Ptr<NocRegistry> instance = factory.Create ()->GetObject<NocRegistry> ();
    return instance;
  }

  TypeId
  NocRegistry::GetTypeId ()
  {
    static TypeId
        tid = TypeId("ns3::NocRegistry")
            .SetParent<Object> ()
            .AddConstructor<NocRegistry> ()
            .AddAttribute (
                "DataPacketSpeedup",
                "How many times a data packet is routed faster than a head packet",
                IntegerValue (1),
                MakeIntegerAccessor (&NocRegistry::m_dataFlitSpeedup),
                MakeIntegerChecker<int> (1))
            .AddAttribute (
                "GlobalClock",
                "The global clock used by a synchronous NoC",
                TimeValue (PicoSeconds (0)),
                MakeTimeAccessor (&NocRegistry::m_globalClock),
                MakeTimeChecker ())
            ;
    return tid;
  }

} // namespace ns3

