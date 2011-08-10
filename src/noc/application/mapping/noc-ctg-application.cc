/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010
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

#include "noc-ctg-application.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/noc-node.h"
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

NS_LOG_COMPONENT_DEFINE ("NocCtgApplication");

using namespace std;

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocCtgApplication);

  TypeId
  NocCtgApplication::GetTypeId ()
  {
    static TypeId
        tid = TypeId ("ns3::NocCtgApplication")
            .SetParent<Application> ()
            .AddConstructor<NocCtgApplication> ()
            .AddAttribute ("Period", "The period of the CTG", TimeValue (Seconds (0)),
                MakeTimeAccessor (&NocCtgApplication::m_period),
                MakeTimeChecker ())
            .AddAttribute ("Iterations", "How many times the CTG is iterated, with the specified period", UintegerValue (1),
                MakeUintegerAccessor (&NocCtgApplication::m_iterations),
                MakeUintegerChecker<uint64_t> (1))
            .AddAttribute ("HSize", "The horizontal size of a 2D mesh (how many nodes can be put on a line)."
                " The vertical size of the 2D mesh is given by number of nodes", UintegerValue (4),
                MakeUintegerAccessor (&NocCtgApplication::m_hSize),
                MakeUintegerChecker<uint32_t> (2))
            .AddAttribute ("NumberOfFlits", "The number of flits composing a packet.", UintegerValue (3),
                MakeUintegerAccessor (&NocCtgApplication::m_numberOfFlits),
                MakeUintegerChecker<uint64_t> (2))
            .AddAttribute ("MaxBytes",
                "The total number of bytes to send. Once these bytes are sent, "
                "no flit is sent again. The value zero means that there is no limit. "
                "Note that if you also set MaxFlits, both constraints must be met for the application to stop.",
                UintegerValue (0), MakeUintegerAccessor (&NocCtgApplication::m_maxBytes),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute ("MaxFlits",
                "The maximum number of flits (head and data) that are injected. "
                "The value zero means that there is no limit. "
                "Note that if you also set MaxBytes, both constraints must be met for the application to stop.",
                UintegerValue (0), MakeUintegerAccessor (&NocCtgApplication::m_maxFlits),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute ("WarmupCycles",
                "How many warmup cycles are considered. During warmup cycles, no statistics are collected",
                UintegerValue (0), MakeUintegerAccessor (&NocCtgApplication::m_warmupCycles),
                MakeUintegerChecker<uint32_t> ())
            .AddTraceSource ("FlitInjected", "A new flit is created and sent",
                MakeTraceSourceAccessor (&NocCtgApplication::m_flitInjectedTrace))
            .AddTraceSource ("PacketInjected", "A new packet was injected into the network",
                MakeTraceSourceAccessor (&NocCtgApplication::m_packetInjectedTrace))
            .AddTraceSource ("FlitReceived", "A flit reached its destination",
                MakeTraceSourceAccessor (&NocCtgApplication::m_flitReceivedTrace))
            ;
    return tid;
  }

  NocCtgApplication::NocCtgApplication ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    IntegerValue flitSize;
    NocRegistry::GetInstance ()->GetAttribute ("FlitSize", flitSize);
    m_flitSize = flitSize.Get () / 8; // in bytes

    m_currentIteration = 0;
    m_totalExecTime = Seconds (0);
    m_executionAvailabilityTime = Seconds (0);
    m_totalData = 0;
  }

  NocCtgApplication::~NocCtgApplication ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NocCtgApplication::FlitReceivedCallback (std::string path, Ptr<const Packet> packet)
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

    uint32_t dataSize = packet->GetSize ();
    NocPacketTag tag;
    packet->PeekPacketTag (tag);
    if (NocPacket::HEAD == tag.GetPacketType ())
      {
        NocHeader header;
        packet->PeekHeader (header);
        dataSize -= header.GetSerializedSize ();
      }

    uint64_t packetIteration = tag.GetCtgIteration ();
    NS_LOG_LOGIC ("Received packet is for CTG iteration " << packetIteration
    		<< " (CTG iterations are numbered starting from 0). This node is currently at CTG iteration " << m_currentIteration);

	NS_ASSERT_MSG (packetIteration < m_iterations, "This packet's CTG iteration exceeds the number of CTG iterations set: " << m_iterations);

    m_receivedData[packetIteration] += dataSize * 8;

    NS_LOG_DEBUG ("For packet CTG iteration, current received data is " << m_receivedData[packetIteration] << ". Total data to be received is " << m_totalData);

    if (m_currentIteration < m_iterations)
      {
        NS_LOG_DEBUG ("For this core's current CTG iteration, current received data is " << m_receivedData[m_currentIteration] << ". Total data to be received is " << m_totalData);

        if (m_receivedData[m_currentIteration] >= m_totalData)
          {
            m_receivedData[m_currentIteration] = m_totalData;

            NS_LOG_INFO ("Node " << GetNode ()->GetId () << " received " << m_totalData << " bits of data for CTG iteration " << m_currentIteration
                << ". Since this is the amount of data expected, node " << GetNode ()->GetId () << " can start injecting flits.");

            if (m_localTaskList.size() == 0)
              {
                NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " does not inject data into the NoC. It will process this flit after a delay of "
					<< m_totalExecTime << " (it's core execution time)");
                Time startTime = Simulator::Now ();
                if (Simulator::Now () < m_executionAvailabilityTime)
                  {
                    NS_LOG_LOGIC ("Simulation time is " << Simulator::Now ()
                            << ". However, this core is still executing data until time "
                            << m_executionAvailabilityTime << ". Its next execution will be scheduled only at that time (not now).");
                    startTime = m_executionAvailabilityTime;
                  }
                  NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " will process the data at time " << m_totalExecTime + startTime);
                  Simulator::Schedule (m_totalExecTime + startTime - Simulator::Now (), &NocCtgApplication::ProcessFlit, this);
                  m_executionAvailabilityTime = m_totalExecTime + startTime;
              }
            else
              {
                // a core with non dependent tasks will be scheduled for injection, for all its tasks, by StartApplication ()
                // thus ScheduleStartEvent (...) must be called here (at receive time) only for cores with non dependent tasks
                if (!ContainsNotDependentTask () && m_currentDestinationIndex[m_currentIteration] < m_localTaskList.size())
                  {
                    ScheduleStartEvent(m_currentIteration);
                  }
                else
                  {
                    NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " already injected all data. No more injections are scheduled.");
                  }
              }
            m_currentIteration++;
          }
    }
    else
      {
        NS_LOG_LOGIC ("This core reached the last CTG iteration. The received packet must be for a previous CTG iteration.");
      }
  }

  void
  NocCtgApplication::ProcessFlit ()
  {
	  NS_LOG_FUNCTION_NOARGS ();
	  NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " finished processing flit");
  }

  void
  NocCtgApplication::SetMaxBytes (uint32_t maxBytes)
  {
    NS_LOG_FUNCTION (this << maxBytes);

    m_maxBytes = maxBytes;
  }

  void
  NocCtgApplication::SetNetDeviceContainer (NetDeviceContainer devices)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_devices = devices;
  }

  void
  NocCtgApplication::SetNodeContainer (NodeContainer nodes)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_nodes = nodes;
  }

  void
  NocCtgApplication::SetTaskList (list<TaskData> taskList)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_taskList = taskList;

    m_totalExecTime = Seconds (0);

    NS_LOG_DEBUG ("Computing the total execution time for NoC node " << GetNode ()->GetId ());
    list<TaskData>::iterator it;
    for (it = m_taskList.begin (); it != m_taskList.end (); it++)
      {
        NS_LOG_DEBUG ("Task " << it->GetId () << " has execution time " << it->GetExecTime ());
        m_totalExecTime += it->GetExecTime ();
      }

    NS_LOG_LOGIC ("Computed a total execution time of " << m_totalExecTime
        << " for the tasks from node " << GetNode ()->GetId ());
  }

  bool
  NocCtgApplication::TaskListContainsTask (string taskId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    bool found = false;

    list<TaskData>::iterator it;
    for (it = m_taskList.begin (); it != m_taskList.end (); it++)
      {
        if (taskId == it->GetId ())
          {
            found = true;
            break;
          }
      }

    return found;
  }

  NocCtgApplication::DependentTaskData
  NocCtgApplication::GetLocalDependentTaskData (uint32_t index)
  {
    NS_LOG_FUNCTION_NOARGS ();

    NS_ASSERT_MSG (index < m_localTaskList.size (), "index = " << index << ", m_localTaskList.size () = " << m_localTaskList.size () << ", node = " << GetNode ()->GetId ());

    uint32_t idx = 0;
    DependentTaskData dtd = *(m_localTaskList.begin ());
    list<DependentTaskData>::iterator it;
    for (it = m_localTaskList.begin (); it != m_localTaskList.end (); it++)
      {
        if (index == idx)
          {
            dtd = *it;
          }
        idx++;
      }

    return dtd;
  }

  bool
  NocCtgApplication::ContainsNotDependentTask ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    bool contains = false;

    for (list<DependentTaskData>::iterator localIt = m_localTaskList.begin(); localIt != m_localTaskList.end(); localIt++)
      {
        string localTaskId = localIt->GetSenderTaskId ();
        bool found = true;
        for (list<DependentTaskData>::iterator remoteIt = m_remoteTaskList.begin(); remoteIt != m_remoteTaskList.end(); remoteIt++)
          {
            if (localIt->GetSenderTaskId () == remoteIt->GetReceivingTaskId ())
              {
                found = false;
                break;
              }
          }
        if (found)
          {
            contains = true;
            NS_LOG_LOGIC ("Found that task " << localTaskId << " is a non dependent task");
            break;
          }
      }

    if (!contains)
      {
        NS_LOG_LOGIC ("All tasks are dependent");
      }

    return contains;
  }

  void
  NocCtgApplication::SetRemoteTaskList (list<DependentTaskData> remoteTaskList)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_remoteTaskList = remoteTaskList;

    list<DependentTaskData>::iterator it;
    for (it = m_remoteTaskList.begin (); it != m_remoteTaskList.end (); it++)
      {
        if (!TaskListContainsTask (it->GetReceivingTaskId ()))
          {
            NS_LOG_ERROR ("Task " << it->GetSenderTaskId () << " has " << it->GetData ()
                << " bytes of data to send to task " << it->GetReceivingTaskId ()
                << ". However, this task is not in the task list!");
          }
        else
          {
            m_totalData += it->GetData ();
          }
      }

    NS_LOG_INFO ("The total amount of data to be received by node " << GetNode ()->GetId () << " is " << m_totalData << " bits.");
  }

  void
  NocCtgApplication::SetLocalTaskList (list<DependentTaskData> localTaskList)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_localTaskList = localTaskList;

    list<DependentTaskData>::iterator it;
    for (it = m_localTaskList.begin (); it != m_localTaskList.end (); it++)
      {
        if (!TaskListContainsTask (it->GetSenderTaskId ()))
          {
            NS_LOG_ERROR ("Task " << it->GetSenderTaskId () << " has " << it->GetData ()
                << " bytes of data to send to task " << it->GetReceivingTaskId ()
                << ". However, this task is not in the task list!");
          }
      }
  }

  void
  NocCtgApplication::DoDispose ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Application::DoDispose();
  }

  // Application Methods
  void
  NocCtgApplication::StartApplication () // Called at time specified by Start
  {
    NS_LOG_LOGIC ("Trying to start injecting flits by checking CTG dependencies (node " << GetNode ()->GetId () << ")");
    NS_LOG_LOGIC ("The CTG will be iterated " << m_iterations << " times, with a period of " << m_period);

    uint32_t nodeId = GetNode ()->GetId ();
    NS_LOG_DEBUG ("Tracing the flits received at node " << (int) nodeId);
    // we configure this trace here and not in the constructor, because
    // the node is not initialized yet at constructor time
    std::stringstream ss;
    ss << "/NodeList/" << nodeId << "/DeviceList/*/$ns3::NocNetDevice/Receive";
    Config::Connect (ss.str (), MakeCallback (&NocCtgApplication::FlitReceivedCallback, this));

    for (uint64_t i = 0; i < m_iterations; ++i)
      {
        m_injectionStarted.insert (m_injectionStarted.end (), false);
        m_totBytes.insert (m_totBytes.end (), 0);
        m_totFlits.insert (m_totFlits.end (), 0);
        m_currentFlitIndex.insert (m_currentFlitIndex.end (), 0);
        m_currentHeadFlit.insert (m_currentHeadFlit.end (), 0);
        m_receivedData.insert (m_receivedData.end (), 0);
        m_currentDestinationIndex.insert (m_currentDestinationIndex.end (), 0);
        m_totalTaskBytes.insert (m_totalTaskBytes.end (), 0);
        m_startEvent.insert (m_startEvent.end (), EventId ());
        m_sendEvent.insert (m_sendEvent.end (), EventId ());

        // Ensure no pending event
        CancelEvents (i);

        ScheduleStartEvent (i);
      }
  }

  void
  NocCtgApplication::StopApplication () // Called at time specified by Stop
  {
    NS_LOG_FUNCTION_NOARGS ();

    for (uint64_t i = 0; i < m_iterations; ++i) {
        CancelEvents (i);
    }
  }

  void
  NocCtgApplication::CancelEvents (uint64_t iteration)
  {
    NS_LOG_FUNCTION (iteration);

    Simulator::Cancel (m_sendEvent[iteration]);
    Simulator::Cancel (m_startEvent[iteration]);
  }

  // Event handlers
  void
  NocCtgApplication::StartSending (uint64_t iteration)
  {
    NS_LOG_FUNCTION (iteration);

    ScheduleNextTx (iteration); // Schedule the send flit event
  }

  void
  NocCtgApplication::StopSending (uint64_t iteration)
  {
    NS_LOG_FUNCTION (iteration);
    CancelEvents (iteration);
  }

  Time
  NocCtgApplication::GetGlobalClock () const
  {
    TimeValue timeValue;
    NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
    Time globalClock = timeValue.Get ();
    NS_ASSERT_MSG (!globalClock.IsZero (), "A global clock must be set!");

    return globalClock;
  }

  // Private helpers
  void
  NocCtgApplication::ScheduleNextTx (uint64_t iteration)
  {
    NS_LOG_FUNCTION (iteration);

    if ((m_maxBytes == 0 || (m_maxBytes > 0 && m_totBytes[iteration] < m_maxBytes))
        && (m_maxFlits == 0 || (m_maxFlits > 0 && m_totFlits[iteration] < m_maxFlits)))
      {
        Time globalClock = GetGlobalClock ();
        Time sendAtTime;
        if (m_totBytes[iteration] == 0)
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
            NS_ASSERT_MSG (sendAtTime.IsPositive(), "sendAtTime is negative! sendAtTime = " << sendAtTime
                << "; globalClock = " << globalClock
                << "; clockMultiplier = " << clockMultiplier
                << "; Simulator::Now () = " << Simulator::Now ()
                << " (sendAtTime = globalClock * Scalar (clockMultiplier) - Simulator::Now ())");
          }
        NS_ASSERT_MSG (sendAtTime >= Scalar (0),
            "The next flit injection is scheduled to run at a time less than the current simulation time!");
        NS_LOG_DEBUG ("Schedule event (flit injection) to occur at time "
            << Simulator::Now () + sendAtTime);
        // Simulator::Schedule (...) receives a relative time
        m_sendEvent[iteration] = Simulator::Schedule (sendAtTime, &NocCtgApplication::SendFlit, this, iteration);
      }
    else
      { // All done, cancel any pending events
        NS_LOG_DEBUG ("Stopping the application");
        NS_LOG_DEBUG ("maxBytes = " << m_maxBytes << " totBytes = " << m_totBytes[iteration]);
        NS_LOG_DEBUG ("maxFlits = " << m_maxFlits << " totFlits = " << m_totFlits[iteration]);

        StopApplication();
      }
  }

  void
  NocCtgApplication::ScheduleStartEvent (uint64_t iteration)
  {
    NS_LOG_FUNCTION (iteration);

    if ((ContainsNotDependentTask () || m_receivedData[iteration] == m_totalData) && m_localTaskList.size() > 0)
    {
        NS_LOG_LOGIC ("Execution time is " << m_totalExecTime);
        NS_LOG_LOGIC ("CTG period is " << m_period);
        NS_LOG_LOGIC ("Current CTG iteration is " << iteration << " (iteration 0 is the first one).");

        Time delay;
        if (!m_injectionStarted[iteration])
          {
            NS_LOG_LOGIC ("This is the first time when node " << GetNode ()->GetId () << " injects flits from CTG iteration " << iteration);
            // the execution of an IP core is simulated by introducing a delay
            // (only before the first flit is injected into the network)
            NS_LOG_LOGIC ("Adding the execution time");
            delay += m_totalExecTime;
            if (ContainsNotDependentTask ())
              {
                // periodicity applies only to source nodes (CTG roots)
                NS_LOG_LOGIC ("Adding CTG period because this node is a traffic source");
                delay += m_period * Scalar (iteration);
              }
          }
        else
          {
            NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " continues injecting flits from CTG iteration " << iteration);
          }
        NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " will start injecting flits after a delay of " << delay);

        Time globalClock = GetGlobalClock ();
        Time startTime = Simulator::Now ();
        if (Simulator::Now () < m_executionAvailabilityTime)
        {
        	NS_LOG_LOGIC ("Simulation time is " << Simulator::Now ()
        		<< ". However, this core is still executing data until time "
        		<< m_executionAvailabilityTime << ". Its next execution will be scheduled only at that time (not now).");
        	startTime = m_executionAvailabilityTime;
        }
        uint64_t clockMultiplier = 1 + (uint64_t) ceil ((startTime + delay).GetSeconds ()
            / globalClock.GetSeconds ()); // 1 + current clock cycle
        Time nextClock = globalClock * Scalar (clockMultiplier) - Simulator::Now ();
        NS_ASSERT_MSG (nextClock.IsPositive(), "Next clock is negative! next clock = " << nextClock
            << "; globalClock = " << globalClock
            << "; clockMultiplier = " << clockMultiplier
            << "; Simulator::Now () = " << Simulator::Now ()
            << " (nextClock = globalClock * Scalar (clockMultiplier) - Simulator::Now ())");

        NS_LOG_LOGIC ("The clock cycle when node " << GetNode ()->GetId () << " will start injecting flits is "
            << globalClock * Scalar (clockMultiplier));

        m_injectionStarted[iteration] = true;

        // Simulator::Schedule (...) receives a relative time
        m_startEvent[iteration] = Simulator::Schedule (nextClock, &NocCtgApplication::StartSending, this, iteration);

        m_executionAvailabilityTime = Simulator::Now () + nextClock;
    }
    else
    {
        if (m_totalData == 0)
          {
            NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " doesn't have any data to receive!");
          }
        if (!ContainsNotDependentTask () && m_receivedData[iteration] < m_totalData)
          {
            NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " still has to receive data before being able to inject its data into the NoC!");
          }
        if (m_localTaskList.size() == 0)
          {
            NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " doesn't have any tasks to send data to (local task list is empty)!");
          }
    }
  }

  void
  NocCtgApplication::SendFlit (uint64_t iteration)
  {
    NS_LOG_FUNCTION (iteration);
    NS_LOG_LOGIC ("sending flit at " << Simulator::Now ());
//    NS_ASSERT (m_sendEvent[iteration].IsExpired ());

    Ptr<NocNode> sourceNode = GetNode ()->GetObject<NocNode> ();
    uint32_t sourceNodeId = sourceNode->GetId ();
    uint32_t sourceX = sourceNodeId % m_hSize;
    uint32_t sourceY = sourceNodeId / m_hSize;
    NS_LOG_DEBUG ("source X = " << sourceX);
    NS_LOG_DEBUG ("source Y = " << sourceY);

    DependentTaskData dtd = GetLocalDependentTaskData (m_currentDestinationIndex[iteration]);
    uint32_t destinationNodeId = dtd.GetReceivingNodeId ();
    uint32_t destinationX = destinationNodeId % m_hSize;
    uint32_t destinationY = destinationNodeId / m_hSize;
    NS_LOG_DEBUG ("destination X = " << destinationX);
    NS_LOG_DEBUG ("destination Y = " << destinationY);

    uint64_t upperValue = (uint64_t)ceil((dtd.GetData() / 8) / m_flitSize);
    // the following test is to account for the smaller payload carried by the head flit
    if ((upperValue - 1) * m_flitSize + m_flitSize - NocHeader::GetHeaderSize() < dtd.GetData() / 8)
      {
        // add another data flit
        upperValue++;
      }

    NS_LOG_DEBUG (dtd.GetData () / 8 << " bytes to send. Flit size is " << m_flitSize
        << " bytes. Therefore, the maximum number of flits per packet is " << upperValue);

    if (m_numberOfFlits > upperValue)
      {
        m_numberOfFlits = upperValue;
      }
    NS_ASSERT_MSG (m_numberOfFlits >= 1, "The number of flits must be at least 1 (the head flit) but it is " << m_numberOfFlits);

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

        if (m_currentDestinationIndex[iteration] < m_localTaskList.size () - 1)
        {
            m_currentDestinationIndex[iteration]++;
            ScheduleNextTx (iteration);
        }
      }
    else
      {
        NS_LOG_LOGIC ("A flit is sent from node " << sourceNodeId << " to node " << destinationNodeId);

        PointerValue nocPointer;
        NocRegistry::GetInstance ()->GetAttribute ("NoCTopology", nocPointer);
        Ptr<NocTopology> nocTopology = nocPointer.Get<NocTopology> ();
        NS_ASSERT_MSG (nocTopology != 0, "The NoC topology was not registered in NocRegistry!");
        vector<uint8_t> relativePositions = nocTopology->GetDestinationRelativeDimensionalPosition (sourceNodeId,
            destinationNodeId);
        uint8_t relativeX = relativePositions[0];
        uint8_t relativeY = relativePositions[1];
        // end traffic pattern

        NS_ASSERT_MSG (m_numberOfFlits >= 1,
            "The number of flits must be at least 1 (the head flit) but it is " << m_numberOfFlits);
        if (m_currentFlitIndex[iteration] == 0)
          {
            NS_ASSERT_MSG (m_flitSize >= (uint64_t) NocHeader::GetHeaderSize(),
                "The flit size must be at least " << NocHeader::GetHeaderSize()
                << " bytes (the packet header size), but it is " << m_flitSize << "!");
            m_currentHeadFlit[iteration] = Create<NocPacket> (relativeX, relativeY, sourceX,
                sourceY, m_numberOfFlits - 1, m_flitSize - NocHeader::GetHeaderSize());
            NocPacketTag packetTag;
            m_currentHeadFlit[iteration]->RemovePacketTag (packetTag);
            packetTag.SetCtgIteration (iteration);
            m_currentHeadFlit[iteration]->AddPacketTag (packetTag);
            NS_LOG_LOGIC ("Preparing to inject flit " << *m_currentHeadFlit[iteration]);
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_flitInjectedTrace (m_currentHeadFlit[iteration]);
              }
            sourceNode->InjectPacket (m_currentHeadFlit[iteration], destinationNode);
            m_currentFlitIndex[iteration]++;
            m_totBytes[iteration] += m_flitSize - NocHeader::GetHeaderSize();
            m_totalTaskBytes[iteration] += m_flitSize - NocHeader::GetHeaderSize();
          }
        else
          {
            bool isTail = false;
            // the last packet sent might be smaller (i.e. its number of flits is < m_numberOfFlits)
            if (m_currentFlitIndex[iteration] + 1 == m_numberOfFlits || (m_totalTaskBytes[iteration] + m_flitSize) * 8 >= dtd.GetData())
              {
                isTail = true;
                NS_LOG_DEBUG ("About to inject a tail flit");
              }
            else
              {
                NS_LOG_DEBUG ("About to inject a data flit");
              }
            Ptr<NocPacket> dataFlit = Create<NocPacket> (m_currentHeadFlit[iteration]->GetUid (), m_flitSize, isTail);
            NocPacketTag packetTag;
            dataFlit->RemovePacketTag (packetTag);
            packetTag.SetCtgIteration (iteration);
            dataFlit->AddPacketTag (packetTag);
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_flitInjectedTrace (dataFlit);
              }
            sourceNode->InjectPacket (dataFlit, destinationNode);
            m_currentFlitIndex[iteration]++;
            m_totBytes[iteration] += m_flitSize;
            m_totalTaskBytes[iteration] += m_flitSize;
          }
        if (m_currentFlitIndex[iteration] == m_numberOfFlits)
          {
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                NS_LOG_DEBUG ("An entire packet was injected into the network");
                m_packetInjectedTrace (m_currentHeadFlit[iteration]);
              }
            m_currentFlitIndex[iteration] = 0;
          }

        m_totFlits[iteration] ++;

        NS_LOG_LOGIC ("Node " << GetNode ()->GetId () << " sent " << m_totalTaskBytes[iteration] << " bytes (in total), at CTG iteration " << iteration);

        if (m_totalTaskBytes[iteration] * 8 >= dtd.GetData())
        {
            NS_LOG_LOGIC ("All data was sent to the current destination node");
            // the last packet sent might be smaller (i.e. its number of flits is < m_numberOfFlits)
            // this means that it must be traced (m_packetTrace) here because the above tracing will most likely not apply
            if (m_currentFlitIndex[iteration] > 0 && m_currentFlitIndex[iteration] < m_numberOfFlits && Simulator::Now () >= GetGlobalClock ()
                * Scalar (m_warmupCycles))
              {
                NS_LOG_DEBUG ("An entire packet was injected into the network (this is the last packet injected and it has a smaller number of flits)");
                m_packetInjectedTrace (m_currentHeadFlit[iteration]);
              }
            m_currentFlitIndex[iteration] = 0;
            // when m_currentDestinationIndex[iteration] = m_localTaskList.size () no further injection will be done
            // (this core injected all the data in the NoC, from all its tasks)
            m_currentDestinationIndex[iteration]++;
            m_totalTaskBytes[iteration] = 0;
        }
        if (m_currentDestinationIndex[iteration] < m_localTaskList.size ())
        {
            ScheduleNextTx (iteration);
        }
      }
  }

  NocCtgApplication::TaskData::TaskData (string id, Time execTime)
  {
    m_id = id;
    m_execTime = execTime;
  }

  string
  NocCtgApplication::TaskData::GetId ()
  {
    return m_id;
  }

  Time
  NocCtgApplication::TaskData::GetExecTime ()
  {
    return m_execTime;
  }

  NocCtgApplication::DependentTaskData::DependentTaskData (string senderTaskId,
		uint32_t senderNodeId, double data, string receivingTaskId,
		uint32_t receivingNodeId)
  {
	m_senderTaskId = senderTaskId;
	m_senderNodeId = senderNodeId;
	m_data = data;
	m_receivingTaskId = receivingTaskId;
	m_receivingNodeId = receivingNodeId;
  }

  string
  NocCtgApplication::DependentTaskData::GetSenderTaskId ()
  {
    return m_senderTaskId;
  }

  uint32_t
  NocCtgApplication::DependentTaskData::GetSenderNodeId ()
  {
    return m_senderNodeId;
  }

  double
  NocCtgApplication::DependentTaskData::GetData ()
  {
    return m_data;
  }

  string
  NocCtgApplication::DependentTaskData::GetReceivingTaskId ()
  {
    return m_receivingTaskId;
  }

  uint32_t
  NocCtgApplication::DependentTaskData::GetReceivingNodeId ()
  {
    return m_receivingNodeId;
  }

} // Namespace ns3
