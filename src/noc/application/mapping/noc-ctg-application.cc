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
            .AddAttribute ("HSize", "The horizontal size of a 2D mesh (how many nodes can be put on a line)."
                " The vertical size of the 2D mesh is given by number of nodes", UintegerValue (4),
                MakeUintegerAccessor (&NocCtgApplication::m_hSize),
                MakeUintegerChecker<uint32_t> (2))
            .AddAttribute("FlitSize", "The flit size, in bytes "
                "(the head flit will use part of this size for the packet header).", UintegerValue (32),
                MakeUintegerAccessor(&NocCtgApplication::m_flitSize),
                MakeUintegerChecker<uint32_t> ((uint32_t) NocHeader::HEADER_SIZE))
            .AddAttribute ("NumberOfFlits", "The number of flits composing a packet.", UintegerValue (3),
                MakeUintegerAccessor (&NocCtgApplication::m_numberOfFlits),
                MakeUintegerChecker<uint32_t> (2))
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

    m_connected = false;
    m_lastStartTime = Seconds (0);
    m_totBytes = 0;
    m_totFlits = 0;
    m_currentFlitIndex = 0;
    m_totalExecTime = Seconds (0);
    m_totalData = 0;
    m_receivedData = 0;
    m_currentDestinationIndex = 0;
    m_totalTaskBytes = 0;
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
    NocHeader header;
    packet->PeekHeader (header);
    if (!header.IsEmpty ())
      {
        dataSize -= header.GetSerializedSize ();
      }

    m_receivedData += dataSize * 8;

    NS_LOG_DEBUG ("Current received data is " << m_receivedData << ". Total data to be received is " << m_totalData);

    if (m_receivedData >= m_totalData)
    {
        m_receivedData = m_totalData;

    	NS_LOG_INFO ("Received " << m_totalData
    			<< " bits of data. Since this is the amount of data expected, this node can start injecting flits.");

    	ScheduleStartEvent();
    }
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

    list<TaskData>::iterator it;
    for (it = m_taskList.begin (); it != m_taskList.end (); it++)
    {
    	m_totalExecTime += it->GetExecTime ();
	}

    NS_LOG_INFO ("Computed a total execution time of " << m_totalExecTime.GetSeconds ()
    		<< " seconds for the tasks from node " << GetNode ()->GetId ());
  }

  bool
  NocCtgApplication::TaskListContainsTask (string taskId)
  {
	NS_LOG_FUNCTION_NOARGS ();

	bool found = false;

	list<TaskData>::iterator it;
	for (it = m_taskList.begin (); it != m_taskList.end (); it++) {
		if (taskId == it->GetId ())
		{
			found = true;
			break;
		}
	}

	return found;
  }

  NocCtgApplication::DependentTaskData
  NocCtgApplication::GetDestinationDependentTaskData (uint32_t index)
  {
	NS_LOG_FUNCTION_NOARGS ();

	NS_ASSERT (index < m_taskDestinationList.size ());

	uint32_t idx = 0;
	DependentTaskData dtd = *(m_taskDestinationList.begin ());
	list<DependentTaskData>::iterator it;
	for (it = m_taskDestinationList.begin (); it != m_taskDestinationList.end (); it++)
	{
		if (index == idx)
		{
			dtd = *it;
		}
		idx++;
	}

	return dtd;
  }

  void
  NocCtgApplication::SetTaskSenderList (list<DependentTaskData> taskSenderList)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_taskSenderList = taskSenderList;

	list<DependentTaskData>::iterator it;
	for (it = m_taskSenderList.begin (); it != m_taskSenderList.end (); it++) {
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

	NS_LOG_INFO ("The total amount of data to be received by this node is " << m_totalData << " bits.");
  }

  void
  NocCtgApplication::SetTaskDestinationList (list<DependentTaskData> taskDestinationList)
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_taskDestinationList = taskDestinationList;

	list<DependentTaskData>::iterator it;
	for (it = m_taskDestinationList.begin (); it != m_taskDestinationList.end (); it++) {
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
    NS_LOG_FUNCTION_NOARGS ();

    uint32_t nodeId = GetNode ()->GetId ();
    NS_LOG_DEBUG ("Tracing the flits received at node " << (int) nodeId);
    // we configure this trace here and not in the constructor, because
    // the node is not initialized yet at constructor time
    std::stringstream ss;
    ss << "/NodeList/" << nodeId << "/DeviceList/*/$ns3::NocNetDevice/Receive";
    Config::Connect (ss.str (), MakeCallback (&NocCtgApplication::FlitReceivedCallback, this));

    // Ensure no pending event
    CancelEvents();
    m_connected = true;

    if (m_connected)
      {
        ScheduleStartEvent();
      }
  }

  void
  NocCtgApplication::StopApplication () // Called at time specified by Stop
  {
    NS_LOG_FUNCTION_NOARGS ();

    CancelEvents();
  }

  void
  NocCtgApplication::CancelEvents ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    Simulator::Cancel(m_sendEvent);
    Simulator::Cancel(m_startEvent);
  }

  // Event handlers
  void
  NocCtgApplication::StartSending ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_lastStartTime = Simulator::Now ();
    ScheduleNextTx (); // Schedule the send flit event
  }

  void
  NocCtgApplication::StopSending ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    CancelEvents();

//    ScheduleStartEvent();
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
  NocCtgApplication::ScheduleNextTx ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    if ((m_maxBytes == 0 || (m_maxBytes > 0 && m_totBytes < m_maxBytes))
        && (m_maxFlits == 0 || (m_maxFlits > 0 && m_totFlits < m_maxFlits)))
      {
//        int speedup = 1;
//        if (m_currentFlitIndex != 0)
//          {
//            // a data flit will be sent
//            IntegerValue dataFlitSpeedup;
//            Ptr<NocRegistry> nocRegistry = NocRegistry::GetInstance ();
//            nocRegistry->GetAttribute ("DataFlitSpeedup", dataFlitSpeedup);
//            speedup = dataFlitSpeedup.Get ();
//            NS_LOG_DEBUG ("Data flit speedup is " << speedup);
//          }
//        else
//          {
//            NS_LOG_DEBUG ("Head flit speedup is " << speedup);
//          }

        Time globalClock = GetGlobalClock ();
//        Time sendAtTime = globalClock / Scalar (speedup);
        Time sendAtTime = globalClock;
        if (m_totBytes == 0)
          {
            // force the first event to occur at time zero
            sendAtTime = Seconds (0);
          }
        NS_LOG_DEBUG ("Schedule event (flit injection) to occur at time "
            << (Simulator::Now () + sendAtTime).GetSeconds () << " seconds");
        m_sendEvent = Simulator::Schedule (sendAtTime, &NocCtgApplication::SendPacket, this);
      }
    else
      { // All done, cancel any pending events
        NS_LOG_DEBUG ("Stopping the application");
        NS_LOG_DEBUG ("maxBytes = " << m_maxBytes << " totBytes = " << m_totBytes);
        NS_LOG_DEBUG ("maxFlits = " << m_maxFlits << " totFlits = " << m_totFlits);

        StopApplication();
      }
  }

  void
  NocCtgApplication::ScheduleStartEvent ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (m_receivedData == m_totalData && m_taskDestinationList.size() > 0)
    {
        NS_LOG_INFO ("Node " << GetNode ()->GetId () << " will start injecting flits after a time of "
            << m_totalExecTime.GetSeconds() << " seconds.");

        m_startEvent = Simulator::Schedule(Simulator::Now() + m_totalExecTime, &NocCtgApplication::StartSending, this);
    }
    else
    {
        if (m_totalData == 0)
          {
            NS_LOG_INFO ("Node " << GetNode ()->GetId () << " doesn't have any data to receive!");
          }
        if (m_receivedData < m_totalData)
          {
            NS_LOG_INFO ("Node " << GetNode ()->GetId () << " still has to receive data before being able to inject its data into the NoC!");
          }
        if (m_taskDestinationList.size() == 0)
          {
            NS_LOG_INFO ("Node " << GetNode ()->GetId () << " doesn't have any tasks to send data to (task destination list is empty)!");
          }
    }
  }

  void
  NocCtgApplication::SendPacket ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    NS_LOG_LOGIC ("sending flit at " << Simulator::Now ());
    NS_ASSERT (m_sendEvent.IsExpired ());

    Ptr<NocNode> sourceNode = GetNode ()->GetObject<NocNode> ();
    uint32_t sourceNodeId = sourceNode->GetId ();
    uint32_t sourceX = sourceNodeId % m_hSize;
    uint32_t sourceY = sourceNodeId / m_hSize;
    NS_LOG_DEBUG ("source X = " << sourceX);
    NS_LOG_DEBUG ("source Y = " << sourceY);

    DependentTaskData dtd = GetDestinationDependentTaskData (m_currentDestinationIndex);
    uint32_t destinationNodeId = dtd.GetReceivingNodeId ();
    uint32_t destinationX = destinationNodeId % m_hSize;
    uint32_t destinationY = destinationNodeId / m_hSize;
    NS_LOG_DEBUG ("destination X = " << destinationX);
    NS_LOG_DEBUG ("destination Y = " << destinationY);

    uint16_t upperValue = (uint16_t)ceil((dtd.GetData() / 8) / m_flitSize);
    // the following test is to account for the smaller payload carried by the head flit
    if ((upperValue - 1) * m_flitSize + m_flitSize - NocHeader::HEADER_SIZE < dtd.GetData() / 8)
      {
        // add another data flit
        upperValue++;
      }

    NS_LOG_DEBUG (dtd.GetData () / 8 << " bytes to send. Flit size is " << m_flitSize
        << ". Therefore, the maximum number of flits is " << upperValue);

    if (m_numberOfFlits > upperValue)
      {
        m_numberOfFlits = upperValue;
      }

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

        if (m_currentDestinationIndex < m_taskDestinationList.size () - 1)
        {
            m_currentDestinationIndex++;
            ScheduleNextTx();
        }
      }
    else
      {
        NS_LOG_LOGIC ("A flit is sent from node " << sourceNodeId << " to node " << destinationNodeId);

        uint32_t relativeX = 0;
        uint32_t relativeY = 0;
        if (destinationX < sourceX)
          {
            // 0 = East; 1 = West
            relativeX = NocHeader::DIRECTION_BIT_MASK;
          }
        if (destinationY < sourceY)
          {
            // 0 = South; 1 = North
            relativeY = NocHeader::DIRECTION_BIT_MASK;
          }
        relativeX = relativeX | std::abs ((int) (destinationX - sourceX));
        relativeY = relativeY | std::abs ((int) (destinationY - sourceY));
        // end traffic pattern

        NS_ASSERT_MSG (m_numberOfFlits >= 1,
            "The number of flits must be at least 1 (the head flit) but it is " << m_numberOfFlits);
        if (m_currentFlitIndex == 0)
          {
            NS_ASSERT_MSG (m_flitSize >= (uint32_t) NocHeader::HEADER_SIZE, "The flit size must be at least " << NocHeader::HEADER_SIZE << " (the flit header size)");
            m_currentHeadFlit = Create<NocPacket> (relativeX, relativeY, sourceX,
                sourceY, m_numberOfFlits - 1, m_flitSize - NocHeader::HEADER_SIZE);
            NS_LOG_LOGIC ("Preparing to inject flit " << *m_currentHeadFlit);
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_flitInjectedTrace (m_currentHeadFlit);
              }
            sourceNode->InjectPacket (m_currentHeadFlit, destinationNode);
            m_currentFlitIndex++;
            m_totBytes += m_flitSize - NocHeader::HEADER_SIZE;
            m_totalTaskBytes += m_flitSize - NocHeader::HEADER_SIZE;
          }
        else
          {
            bool isTail = false;
            // the last packet sent might be smaller (i.e. its number of flits is < m_numberOfFlits)
            if (m_currentFlitIndex + 1 == m_numberOfFlits || (m_totalTaskBytes + m_flitSize) * 8 >= dtd.GetData())
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
            m_totalTaskBytes += m_flitSize;
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
        m_lastStartTime = Simulator::Now ();

        if (m_totalTaskBytes * 8 >= dtd.GetData())
        {
            // the last packet sent might be smaller (i.e. its number of flits is < m_numberOfFlits)
            // this means that it must be traced (m_packetTrace) here because the above tracing will most likely not apply
            if (m_currentFlitIndex > 0 && m_currentFlitIndex < m_numberOfFlits && Simulator::Now () >= GetGlobalClock ()
                * Scalar (m_warmupCycles))
              {
                NS_LOG_DEBUG ("An entire packet was injected into the network (this is the last packet injected and it has a smaller number of flits)");
                m_packetInjectedTrace (m_currentHeadFlit);
              }
            m_currentFlitIndex = 0;
            m_currentDestinationIndex++;
            m_totalTaskBytes = 0;
        }
        if (m_currentDestinationIndex < m_taskDestinationList.size ())
        {
            ScheduleNextTx();
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
