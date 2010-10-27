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
            .AddAttribute("PacketSize", "The size of data packets sent (in Bytes). "
                "For head packets, the size of the header is not included.", UintegerValue (512),
                MakeUintegerAccessor(&NocCtgApplication::m_pktSize),
                MakeUintegerChecker<uint32_t> (1))
            .AddAttribute ("NumberOfPackets", "How many packets a message will have.", UintegerValue (3),
                MakeUintegerAccessor (&NocCtgApplication::m_numberOfPackets),
                MakeUintegerChecker<uint32_t> (1))
            .AddAttribute ("MaxBytes",
                "The total number of bytes to send. Once these bytes are sent, "
                "no packet is sent again. The value zero means that there is no limit. "
                "Note that if you also set MaxPackets, both constraints must be met for the application to stop.",
                UintegerValue (0), MakeUintegerAccessor (&NocCtgApplication::m_maxBytes),
                MakeUintegerChecker<uint32_t> ())
            .AddAttribute ("MaxPackets",
                "The maximum number of packets (head and data) that could be injected. "
                "We say 'could' because the injection probability is considered. "
                "The value zero means that there is no limit. "
                "Note that if you also set MaxBytes, both constraints must be met for the application to stop.",
                UintegerValue (0), MakeUintegerAccessor (&NocCtgApplication::m_maxPackets),
                MakeUintegerChecker<uint32_t> ())
                // FIXME is warmup cycles required?
            .AddAttribute ("WarmupCycles",
                "How many warmup cycles are considered. During warmup cycles, no statistics are collected",
                UintegerValue (0), MakeUintegerAccessor (&NocCtgApplication::m_warmupCycles),
                MakeUintegerChecker<uint32_t> ())
            .AddTraceSource ("Tx", "A new packet is created and sent",
                MakeTraceSourceAccessor (&NocCtgApplication::m_txTrace))
            .AddTraceSource ("MessageInjected", "A new message was injected into the network",
                MakeTraceSourceAccessor (&NocCtgApplication::m_messageTrace))
            .AddTraceSource ("PacketReceived", "A packet reached its destination",
                MakeTraceSourceAccessor (&NocCtgApplication::m_packetReceivedTrace))
            ;
    return tid;
  }

  NocCtgApplication::NocCtgApplication ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    m_connected = false;
    m_lastStartTime = Seconds (0);
    m_totBytes = 0;
    m_totPackets = 0;
    m_currentPacketIndex = 0;
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
  NocCtgApplication::PacketReceivedCallback (std::string path, Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION ("path" << path << "packet UID" << packet->GetUid ());

    if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
      {
        NS_LOG_DEBUG ("Tracing the packet");
        m_packetReceivedTrace (packet);
      }
    else
      {
        NS_LOG_DEBUG ("Not tracing the packet");
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
    			<< " bits of data. Since this is the amount of data expected, this node can start injecting packets.");

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
    NS_LOG_DEBUG ("Tracing the packets received at node " << (int) nodeId);
    // we configure this trace here and not in the constructor, because
    // the node is not initialized yet at constructor time
    std::stringstream ss;
    ss << "/NodeList/" << nodeId << "/DeviceList/*/$ns3::NocNetDevice/Receive";
    Config::Connect (ss.str (), MakeCallback (&NocCtgApplication::PacketReceivedCallback, this));

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
    ScheduleNextTx (); // Schedule the send packet event
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
        && (m_maxPackets == 0 || (m_maxPackets > 0 && m_totPackets < m_maxPackets)))
      {
//        int speedup = 1;
//        if (m_currentPacketIndex != 0)
//          {
//            // a data packet will be sent
//            IntegerValue dataFlitSpeedup;
//            Ptr<NocRegistry> nocRegistry = NocRegistry::GetInstance ();
//            nocRegistry->GetAttribute ("DataPacketSpeedup", dataFlitSpeedup);
//            speedup = dataFlitSpeedup.Get ();
//            NS_LOG_DEBUG ("Data packet speedup is " << speedup);
//          }
//        else
//          {
//            NS_LOG_DEBUG ("Head packet speedup is " << speedup);
//          }

        Time globalClock = GetGlobalClock ();
//        Time sendAtTime = globalClock / Scalar (speedup);
        Time sendAtTime = globalClock;
        if (m_totBytes == 0)
          {
            // force the first event to occur at time zero
            sendAtTime = Seconds (0);
          }
        NS_LOG_DEBUG ("Schedule event (packet injection) to occur at time "
            << (Simulator::Now () + sendAtTime).GetSeconds () << " seconds");
        m_sendEvent = Simulator::Schedule (sendAtTime, &NocCtgApplication::SendPacket, this);
      }
    else
      { // All done, cancel any pending events
        NS_LOG_DEBUG ("Stopping the application");
        NS_LOG_DEBUG ("maxBytes = " << m_maxBytes << " totBytes = " << m_totBytes);
        NS_LOG_DEBUG ("maxPackets = " << m_maxPackets << " totPackets = " << m_totPackets);

        StopApplication();
      }
  }

  void
  NocCtgApplication::ScheduleStartEvent ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    if (m_receivedData == m_totalData && m_taskDestinationList.size() > 0)
    {
        NS_LOG_INFO ("Node " << GetNode ()->GetId () << " will start injecting packets after a time of "
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
    NS_LOG_LOGIC ("sending packet at " << Simulator::Now ());
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

    uint16_t upperValue = (uint16_t)ceil((dtd.GetData() / 8) / m_pktSize);

    NS_LOG_DEBUG (dtd.GetData () / 8 << " bytes to send. Packet size is " << m_pktSize
        << ". Therefore, the maximum number of packets is " << upperValue);

    if (m_numberOfPackets > upperValue)
      {
        m_numberOfPackets = upperValue;
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
        NS_LOG_LOGIC ("Trying to send a message from node " << sourceNodeId << " to node "
            << destinationNodeId << ". Aborting because source and destination nodes are the same.");

        if (m_currentDestinationIndex < m_taskDestinationList.size () - 1)
        {
        	m_currentDestinationIndex++;
        	ScheduleNextTx();
        }
      }
    else
      {
        NS_LOG_LOGIC ("A packet is sent from node " << sourceNodeId << " to node " << destinationNodeId);

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

        NS_ASSERT_MSG (m_numberOfPackets >= 1,
            "The number of packets must be at least 1 (the head packet) but it is " << m_numberOfPackets);
        if (m_currentPacketIndex == 0)
          {
            m_currentHeadPacket = Create<NocPacket> (relativeX, relativeY, sourceX,
                sourceY, m_numberOfPackets - 1, m_pktSize);
            NS_LOG_LOGIC ("Preparing to inject packet " << *m_currentHeadPacket);
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_txTrace (m_currentHeadPacket);
              }
            sourceNode->InjectPacket (m_currentHeadPacket, destinationNode);
            m_currentPacketIndex++;
          }
        else
          {
            bool isTail = false;
            if (m_currentPacketIndex + 1 == m_numberOfPackets)
              {
                isTail = true;
                NS_LOG_DEBUG ("About to inject a tail packet");
              }
            else
              {
                NS_LOG_DEBUG ("About to inject a data packet");
              }
            Ptr<NocPacket> dataPacket = Create<NocPacket> (m_currentHeadPacket->GetUid (), m_pktSize, isTail);
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_txTrace (dataPacket);
              }
            sourceNode->InjectPacket (dataPacket, destinationNode);
            m_currentPacketIndex++;
          }
        if (m_currentPacketIndex == m_numberOfPackets)
          {
            if (Simulator::Now () >= GetGlobalClock () * Scalar (m_warmupCycles))
              {
                m_messageTrace (m_currentHeadPacket);
              }
              m_currentPacketIndex = 0;
          }

        m_totBytes += m_pktSize;
        m_totPackets ++;
        m_lastStartTime = Simulator::Now ();
        m_totalTaskBytes += m_pktSize;

        if (m_totalTaskBytes * 8 >= dtd.GetData())
        {
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
