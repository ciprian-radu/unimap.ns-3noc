/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *               						Lucian Blaga University of Sibiu, Romania
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
 * Author: Ciprian Radu <ciprian.radu@ulbsibiu.ro>
 */

#include "noc-sync-application.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/noc-node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/noc-packet.h"
#include "ns3/noc-net-device.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/noc-registry.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/enum.h"
#include <cstdlib>
#include <bitset>
#include "stdio.h"
#include "ns3/config.h"
#include "ns3/noc-packet-tag.h"
#include "ns3/pointer.h"
#include "ns3/noc-value.h"
#include "ns3/object-vector.h"

NS_LOG_COMPONENT_DEFINE ("NocSyncApplication");

using namespace std;

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocSyncApplication);

  std::string
  NocSyncApplication::TrafficPatternToString (TrafficPattern t)
  {
    switch (t)
      {
        case DESTINATION_SPECIFIED:
          return "DestinationSpecified";

        case UNIFORM_RANDOM:
          return "UniformRandom";

        case BIT_MATRIX_TRANSPOSE:
          return "BitMatrixTranspose";

        case BIT_COMPLEMENT:
          return "BitComplement";

        case BIT_REVERSE:
          return "BitReverse";

        default:
          return "InvalidTrafficPattern";
      }
  }

  NocSyncApplication::TrafficPattern
  NocSyncApplication::TrafficPatternFromString (std::string t)
  {
    if (t == "DestinationSpecified")
      {
        return DESTINATION_SPECIFIED;
      }
    if (t == "UniformRandom")
      {
        return UNIFORM_RANDOM;
      }
    if (t == "BitMatrixTranspose")
      {
        return BIT_MATRIX_TRANSPOSE;
      }
    if (t == "BitComplement")
      {
        return BIT_COMPLEMENT;
      }
    if (t == "BitReverse")
      {
        return BIT_REVERSE;
      }
    NS_LOG_WARN ("The traffic pattern called '" << t << "' is unknown! "
        "Using the default traffic pattern (Uniform random)");
    return UNIFORM_RANDOM;
  }

  TypeId
  NocSyncApplication::GetTypeId(void)
  {
    static TypeId
        tid = TypeId("ns3::NocSyncApplication")
            .SetParent<Application> ()
            .AddConstructor<NocSyncApplication> ()
            .AddAttribute ("InjectionProbability", "The injection probability",
                DoubleValue (0.5), MakeDoubleAccessor (&NocSyncApplication::m_injectionProbability),
                MakeDoubleChecker<double> (0, 1))
            .AddAttribute ("Size",
                "how many nodes the nD mesh will have on each line", ObjectVectorValue(),
                MakeObjectVectorAccessor (&NocSyncApplication::m_size),
                MakeObjectVectorChecker<NocValue> ())
            .AddAttribute ("NumberOfFlits", "The number of flits composing a packet.", UintegerValue (3),
                MakeUintegerAccessor (&NocSyncApplication::m_numberOfFlits),
                MakeUintegerChecker<uint32_t> (2))
            .AddAttribute("MaxBytes",
                "The total number of bytes to send. Once these bytes are sent, "
                "no flit is sent again. The value zero means that there is no limit. "
                "Note that if you also set MaxFlits, both constraints must be met for the application to stop.",
                UintegerValue(0), MakeUintegerAccessor(&NocSyncApplication::m_maxBytes),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute ("MaxFlits",
                "The maximum number of flits (head and data) that are injected. "
                "The value zero means that there is no limit. "
                "Note that if you also set MaxBytes, both constraints must be met for the application to stop.",
                UintegerValue (0), MakeUintegerAccessor (&NocSyncApplication::m_maxFlits),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute("WarmupCycles",
                "How many warmup cycles are considered. During warmup cycles, no statistics are collected",
                UintegerValue(0), MakeUintegerAccessor(&NocSyncApplication::m_warmupCycles),
                MakeUintegerChecker<uint32_t> ())
            .AddTraceSource("FlitInjected", "A flit packet is created and sent",
                MakeTraceSourceAccessor(&NocSyncApplication::m_flitInjectedTrace))
            .AddTraceSource("PacketInjected", "A new packet was injected into the network",
                MakeTraceSourceAccessor(&NocSyncApplication::m_packetInjectedTrace))
            .AddTraceSource("FlitReceived", "A flit reached its destination",
                MakeTraceSourceAccessor(&NocSyncApplication::m_flitReceivedTrace))
            .AddAttribute("TrafficPattern",
                "The traffic pattern which will be used by this application",
                EnumValue (BIT_COMPLEMENT), MakeEnumAccessor (&NocSyncApplication::m_trafficPatternEnum),
                MakeEnumChecker (DESTINATION_SPECIFIED, TrafficPatternToString(DESTINATION_SPECIFIED),
                                 UNIFORM_RANDOM, TrafficPatternToString(UNIFORM_RANDOM),
                                 BIT_MATRIX_TRANSPOSE, TrafficPatternToString(BIT_MATRIX_TRANSPOSE),
                                 BIT_COMPLEMENT, TrafficPatternToString(BIT_COMPLEMENT),
                                 BIT_REVERSE, TrafficPatternToString(BIT_REVERSE)))
            .AddAttribute("Destination", "The ID of the destination node "
                "(must be specified for a destination specified traffic pattern)", UintegerValue(0),
                MakeUintegerAccessor(&NocSyncApplication::m_destinationNodeId),
                MakeUintegerChecker<uint32_t> (0))
            ;
    return tid;
  }

  NocSyncApplication::NocSyncApplication()
  {
    NS_LOG_FUNCTION_NOARGS ();

    IntegerValue flitSize;
    NocRegistry::GetInstance ()->GetAttribute ("FlitSize", flitSize);
    m_flitSize = flitSize.Get () / 8; // in bytes
    m_totBytes = 0;
    m_totFlits = 0;
    m_trafficPatternEnum = BIT_COMPLEMENT;
    m_currentFlitIndex = 0;
  }

  NocSyncApplication::~NocSyncApplication()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NocSyncApplication::FlitReceivedCallback (std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION ("path" << path << "packet UID" << packet->GetUid ());
    if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
      {
        NS_LOG_DEBUG ("Tracing the flit");
        m_flitReceivedTrace (packet);
      }
    else
      {
        NS_LOG_DEBUG ("Not tracing the flit (warmup period)");
      }
  }

  void
  NocSyncApplication::SetDimensionSize (std::vector<Ptr<NocValue> > size)
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_size=size;
  }
  void
  NocSyncApplication::SetMaxBytes(uint32_t maxBytes)
  {
    NS_LOG_FUNCTION (this << maxBytes);
    m_maxBytes = maxBytes;
  }

  void NocSyncApplication::SetNetDeviceContainer (NetDeviceContainer devices)
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_devices = devices;
  }

  void NocSyncApplication::SetNodeContainer (NodeContainer nodes)
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_nodes = nodes;
  }

  void
  NocSyncApplication::DoDispose ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Application::DoDispose();
  }

  // Application Methods
  void
  NocSyncApplication::StartApplication() // Called at time specified by Start
  {
    NS_LOG_LOGIC ("Starting the application at time " << Simulator::Now ());

    uint32_t nodeId = GetNode ()->GetId ();
    NS_LOG_DEBUG ("Tracing the flits received at node " << (int) nodeId);
    // we configure this trace here and not in the constructor, because
    // the node is not initialized yet at constructor time
    std::stringstream ss;
    ss << "/NodeList/" << nodeId << "/DeviceList/*/$ns3::NocNetDevice/Receive";
    Config::Connect (ss.str (), MakeCallback (&NocSyncApplication::FlitReceivedCallback, this));

    NS_LOG_DEBUG ("Using the " << TrafficPatternToString(m_trafficPatternEnum) << " traffic pattern");

    // Ensure no pending event
    CancelEvents();

    ScheduleStartEvent();
  }

  void
  NocSyncApplication::StopApplication () // Called at time specified by Stop
  {
    NS_LOG_FUNCTION_NOARGS ();

    CancelEvents();
  }

  void
  NocSyncApplication::CancelEvents ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Cancel (m_sendEvent);
    Simulator::Cancel (m_startEvent);
  }

  // Event handlers
  void
  NocSyncApplication::StartSending ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    ScheduleNextTx (); // Schedule the send flit event
  }

  void
  NocSyncApplication::StopSending ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    CancelEvents ();

    ScheduleStartEvent ();
  }

  Time
  NocSyncApplication::GetGlobalClock () const
  {
    TimeValue timeValue;
    NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
    Time globalClock = timeValue.Get ();
    NS_ASSERT_MSG (!globalClock.IsZero (), "A global clock must be set!");

    return globalClock;
  }

  // Private helpers
  void
  NocSyncApplication::ScheduleNextTx ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (m_injectionProbability == 0)
      {
        NS_LOG_INFO ("Stopping the application assigned to node "
            << GetNode ()->GetId () << " because injection probability is set to zero");
        StopApplication();
      }
    else
      {
        if ((m_maxBytes == 0 || (m_maxBytes > 0 && m_totBytes < m_maxBytes))
            && (m_maxFlits == 0 || (m_maxFlits > 0 && m_totFlits < m_maxFlits)))
          {
            Time globalClock = GetGlobalClock ();
            Time sendAtTime;
            if (m_totBytes == 0)
              {
                // the first flit injection event must occur with no delay
                sendAtTime = PicoSeconds (0);
              }
            else
              {
                // find the next network clock cycle
                uint64_t clockMultiplier = 1 + (uint64_t) ceil (Simulator::Now ().GetSeconds ()
                    / globalClock.GetSeconds ()); // 1 + current clock cycle
                sendAtTime = globalClock * Scalar (clockMultiplier) - Simulator::Now ();
                NS_LOG_DEBUG ("clockMultiplier " << clockMultiplier);
                NS_LOG_DEBUG ("globalClock " << globalClock);
                NS_LOG_DEBUG ("sendAtTime " << sendAtTime);
                NS_ASSERT_MSG (sendAtTime.IsPositive(), "sendAtTime is negative! sendAtTime = " << sendAtTime
                    << "; globalClock = " << globalClock
                    << "; clockMultiplier = " << clockMultiplier
                    << "; Simulator::Now () = Simulator::Now () "
                    << "(sendAtTime = globalClock * Scalar (clockMultiplier) - Simulator::Now ())");
              }
            NS_ASSERT_MSG (sendAtTime >= Scalar (0),
               "The next flit injection is scheduled to run at a time less than the current simulation time!");
            NS_LOG_DEBUG ("Schedule event (flit injection) to occur at time "
                << Simulator::Now () + sendAtTime);
            // Simulator::Schedule (...) receives a relative time
            m_sendEvent = Simulator::Schedule (sendAtTime, &NocSyncApplication::SendFlit, this);
          }
        else
          { // All done, cancel any pending events
            NS_LOG_DEBUG ("Stopping the application");
            NS_LOG_DEBUG ("maxBytes = " << m_maxBytes << " totBytes = " << m_totBytes);
            NS_LOG_DEBUG ("maxFlits = " << m_maxFlits << " totFlits = " << m_totFlits);
            StopApplication();
          }
      }
  }

  void
  NocSyncApplication::ScheduleStartEvent ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_startEvent = Simulator::ScheduleNow (&NocSyncApplication::StartSending, this);
  }

  void
  NocSyncApplication::SendFlit ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    NS_LOG_LOGIC ("sending flit at " << Simulator::Now ());
    NS_ASSERT (m_sendEvent.IsExpired ());

    Ptr<NocNode> sourceNode = GetNode ()->GetObject<NocNode> ();
    uint32_t sourceNodeId = sourceNode->GetId ();

    std::vector <uint8_t> source=vector<uint8_t> ();
    uint32_t numberOfNodes=1;

    for (unsigned int k = 0; k < m_size.size (); k++)
      {
        if (m_size.at (k)->GetValue () != 0)
          {
            if (k != 0)
              {
                numberOfNodes *= m_size.at (k - 1)->GetValue();
              }
            else
              {
                numberOfNodes = 1;
              }
            source.insert(source.end(), sourceNodeId / numberOfNodes % m_size.at (k)->GetValue());
          }
        else
          {
            source.at (k) = 0;
          }
        NS_LOG_DEBUG ("dimension " << (k+1) << " source " << (int) source.at (k));
      }

    std::vector <uint32_t> destination=vector<uint32_t> ();

    double log = 0;
    std::vector<uint8_t> size = vector<uint8_t> ();
    for (unsigned int k = 0; k < m_size.size (); k++)
      {
        if (m_size.at (k)->GetValue() > 0)
          {
            log = log2 (m_size.at (k)->GetValue());
          }
        size.insert (size.end (), (uint8_t) floor (log));
        NS_LOG_DEBUG ("dimension " << (k+1) << " size " << (int) size.at (k));
      }

    numberOfNodes=1;

    switch (m_trafficPatternEnum)
      {
        case DESTINATION_SPECIFIED:
          for (unsigned int k = 0; k < m_size.size (); k++)
            {
              if (m_size.at (k)->GetValue () != 0)
                {
                  if (k != 0)
                    {
                      numberOfNodes *= m_size.at (k - 1)->GetValue();
                    }
                  else
                    {
                      numberOfNodes = 1;
                    }
                  destination.insert (destination.end(), m_destinationNodeId / numberOfNodes % m_size.at (k)->GetValue());
                }
              else
                {
                  source.at (k) = 0;
                }NS_LOG_DEBUG ("dimension "<<(k+1)<<" specified destination " <<(int) destination.at(k)); NS_ASSERT (destination.at(k) < m_size.at(k)->GetValue());
            }
      break;
        case UNIFORM_RANDOM:
          if (m_currentFlitIndex == 0)
            {
              for (unsigned int k = 0; k < m_size.size (); k++)
                {
                  if (m_uniformDestination.size () <= k)
                    {
                      m_uniformDestination.insert (m_uniformDestination.end(), m_trafficPattern.GetUniformRandomNumber (0,
                          m_size.at (k)->GetValue() - 1));
                    }
                  else
                    {
                      m_uniformDestination.at (k) = m_trafficPattern.GetUniformRandomNumber (0,
                          m_size.at (k)->GetValue() - 1);
                    }
                }
            }
          for (unsigned int k = 0; k < m_size.size (); k++)
            {
              destination.insert (destination.end (), m_uniformDestination.at (k));
              NS_LOG_DEBUG("dimension "<<(k+1)<<" random destination = "<<(int) destination.at(k));
              NS_ASSERT (destination.at(k) < m_size.at(k)->GetValue());
            }
          break;

        case BIT_MATRIX_TRANSPOSE:
          for (unsigned int k = 0; k < m_size.size (); k++)
            {
              destination.insert(destination.end(), ns3::TrafficPattern::MatrixTransposeBits(source.at(k), size.at(k)));
              NS_ASSERT (destination.at(k) < m_size.at(k)->GetValue());
            }
          break;

        case BIT_COMPLEMENT:
          for (unsigned int k = 0; k < m_size.size (); k++)
            {
              destination.insert (destination.end (), ns3::TrafficPattern::ComplementBits(source.at (k), size.at (k)));
              NS_ASSERT (destination.at(k) < m_size.at(k)->GetValue());
            }
          break;

        case BIT_REVERSE:
          for (unsigned int k = 0; k < m_size.size (); k++)
           {
             destination.insert(destination.end(), ns3::TrafficPattern::ReverseBits(source.at(k), size.at(k)));
             NS_ASSERT (destination.at(k) < m_size.at(k)->GetValue());
           };
          break;

        default:
          break;
      }

    uint32_t destinationNodeId=0;
    numberOfNodes=1;
    for (unsigned int k = 0; k < m_size.size (); k++)
      {
        if (m_size.at (k)->GetValue () != 0)
          {
            if (k != 0)
              {
                numberOfNodes *= m_size.at (k - 1)->GetValue ();
              }
            else
              {
                numberOfNodes = 1;
              }
            destinationNodeId += destination.at (k) * numberOfNodes;
          }
      }

    NS_LOG_DEBUG ("destinationNodeId = " << destinationNodeId);

    Ptr<NocNode> destinationNode;
    for (NetDeviceContainer::Iterator i = m_devices.Begin(); i
        != m_devices.End(); ++i)
      {
        Ptr<Node> tmpNode = (*i)->GetNode();
        if (destinationNodeId == tmpNode->GetId())
          {
            destinationNode = tmpNode->GetObject<NocNode> ();
            break;
          }
      }
    if (sourceNodeId == destinationNodeId)
      {
        NS_LOG_LOGIC ("Trying to send a packet from node " << sourceNodeId << " to node "
            << destinationNodeId << ". Aborting because source and destination nodes are the same.");
        if (m_trafficPatternEnum == UNIFORM_RANDOM)
          {
            ScheduleNextTx ();
          }
      }
    else
      {
        NS_LOG_LOGIC ("A flit is sent from node " << sourceNodeId << " to node " << destinationNodeId);

        PointerValue nocPointer;
        NocRegistry::GetInstance ()->GetAttribute ("NoCTopology", nocPointer);
        Ptr<NocTopology> nocTopology = nocPointer.Get<NocTopology> ();
        NS_ASSERT_MSG (nocTopology != 0, "The NoC topology was not registered in NocRegistry!");
        std::vector<uint8_t> relativePositions = nocTopology->GetDestinationRelativeDimensionalPosition (sourceNodeId,
            destinationNodeId);
        std::vector <uint8_t> relative=vector<uint8_t>();

        for (unsigned int k = 0; k < relativePositions.size (); k++)
          {
            relative.insert(relative.end(), relativePositions.at(k));
            NS_LOG_DEBUG ("dimension "<<(k+1)<<" relative = " << (int) relative.at(k));
          }

        NS_ASSERT_MSG (m_numberOfFlits >= 1,
            "The number of flits must be at least 1 (the head flit) but it is " << m_numberOfFlits);
        if (m_currentFlitIndex == 0)
          {
            int randomValue = rand ();
            randomValue = randomValue % 100 + 1; // 1 .. 100
            if (randomValue <= m_injectionProbability * 100)
              {
                NS_LOG_LOGIC ("A new packet is injected into the network (injection probability is "
                    << m_injectionProbability << ")");

                NS_ASSERT_MSG (m_flitSize >= (uint64_t) NocHeader::GetHeaderSize(),
                    "The flit size must be at least " << NocHeader::GetHeaderSize()
                    << " bytes (the packet header size), but it is " << m_flitSize << "!");
                m_currentHeadFlit = Create<NocPacket> (relative, source, m_numberOfFlits - 1, (m_flitSize
                    - NocHeader::GetHeaderSize ()));
                NS_LOG_LOGIC ("Preparing to inject packet " << *m_currentHeadFlit);
                if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
                  {
                    m_flitInjectedTrace (m_currentHeadFlit);
                  }
                sourceNode->InjectPacket (m_currentHeadFlit, destinationNode);
                m_currentFlitIndex++;
                m_totBytes += m_flitSize - NocHeader::GetHeaderSize();
                NS_LOG_DEBUG ("m_totBytes " << m_totBytes);
              }
            else
              {
                NS_LOG_LOGIC ("A new packet is not injected into the network (injection probability is "
                    << m_injectionProbability << ")");
              }
          }
        else
          {
            bool isTail = false;
            if (m_currentFlitIndex + 1 == m_numberOfFlits)
              {
                isTail = true;
                NS_LOG_DEBUG ("About to inject a tail flit");
              }
            else
              {
                NS_LOG_DEBUG ("About to inject a data flit");
              }
            Ptr<NocPacket> dataFlit = Create<NocPacket> (m_currentHeadFlit->GetUid (), m_flitSize, isTail);
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_flitInjectedTrace (dataFlit);
              }
            sourceNode->InjectPacket (dataFlit, destinationNode);
            m_currentFlitIndex++;
            m_totBytes += m_flitSize;
            NS_LOG_DEBUG ("m_totBytes " << m_totBytes);
          }
        if (m_currentFlitIndex == m_numberOfFlits)
          {
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                NS_LOG_DEBUG ("An entire packet was injected into the network");              
                m_packetInjectedTrace (m_currentHeadFlit);
              }
              m_currentFlitIndex = 0;
          }

        m_totFlits ++;
        ScheduleNextTx ();
      }
  }

} // Namespace ns3
