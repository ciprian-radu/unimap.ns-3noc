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

#ifndef NOC_CTG_APPLICATION_H
#define NOC_CTG_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/noc-packet.h"
#include "ns3/nstime.h"
#include <vector>

using namespace std;

namespace ns3 {

class Address;
class Socket;

/**
 * \brief Network on Chip (NoC) application which injects flits into
 * the network based on a Communication Task Graph (CTG). The CTG can run for several iterations.
 * This application is to be used with a synchronous NoC: events are
 * generated by the application only, at a certain frequency which represents
 * the network's clock.
 */
class NocCtgApplication : public Application
{
public:

  /**
    * Simple data structure for keeping information about the tasks assigned
    * to this ns-3 application (and implicitly IP core and NoC node).
    */
   class TaskData {

   private:

     /** the ID of the task */
     string m_id;

     /**
      * the execution time of the task (considering that the task is
      * assigned to the IP core which was mapped to the NoC node that
      * has this ns-3 application). It is measured in seconds.
      **/
     Time m_execTime;

   public:

     /**
      * Constructor
      *
      * \param   id the ID of the task
      *
      * \param   the execution time of the task (considering that the task is
      *          assigned to the IP core which was mapped to the NoC node that
      *          has this ns-3 application)
      *
      */
     TaskData (string id, Time execTime);

     /**
      * \return the ID of the task
      */
     string
     GetId ();

     /**
      * \return the execution time of the task (measured in seconds)
      */
     Time
     GetExecTime ();

   };

   /**
    * Simple data structure for modeling a data dependency between
    * two tasks, belonging to different NoC nodes.
    */
   class DependentTaskData {

   private:

     /** the ID of the sender task */
     string m_senderTaskId;

     /** the ID of the node to which the sender task is associated to */
     uint32_t m_senderNodeId;

     /** the amount of communicated data (communication volume), expressed in bits */
     double m_data;

     /** the ID of receiving task */
     string m_receivingTaskId;

     /** the ID of the node to which the receiving task is associated to */
     uint32_t m_receivingNodeId;

   public:

     DependentTaskData (string senderTaskId, uint32_t senderNodeId,
				double data, string receivingTaskId, uint32_t receivingNodeId);

     /**
      * \return the ID of the sender task
      */
     string
     GetSenderTaskId ();

     /**
      * \return the ID of the sender node
      */
     uint32_t
     GetSenderNodeId ();

     /**
      * \return the amount of communicated data, expressed in bits
      */
     double
     GetData ();

     /**
      * \return the ID of the receiving task
      */
     string
     GetReceivingTaskId ();

     /**
      * \return the ID of the receiving node
      */
     uint32_t
     GetReceivingNodeId ();

   };

  static TypeId
  GetTypeId ();

  NocCtgApplication ();

  virtual ~NocCtgApplication ();

  void
  SetMaxBytes (uint32_t maxBytes);

  void
  SetNetDeviceContainer (NetDeviceContainer devices);

  void
  SetNodeContainer (NodeContainer nodes);

  /**
   * Sets the task list and computes the total execution time of these tasks (as the sum of the execution times of each task).
   * This method should be called right after instantiating this NocCtgApplication.
   *
   * \param taskList keeps all the tasks that are assigned to the IP core
   *        associated with this ns-3 application
   */
  void
  SetTaskList (list<TaskData> taskList);

  /**
   * Sets the task sender list. This method should be called right after instantiating
   * this NocCtgApplication.
   *
   * \param keeps all the remote tasks that send data to this NoC node
   */
  void
  SetTaskSenderList (list<DependentTaskData> taskSenderList);

  /**
   * Sets the task destination list. This method should be called right after instantiating
   * this NocCtgApplication.
   *
   * \param keeps all the local tasks that send data to tasks from remote NoC nodes
   */
  void
  SetTaskDestinationList (list<DependentTaskData> taskDestinationList);

protected:

  virtual void
  DoDispose ();

private:

  // inherited from Application base class.
  virtual void
  StartApplication ();    // Called at time specified by Start

  virtual void
  StopApplication ();     // Called at time specified by Stop

  void
  CancelEvents (uint64_t iteration);

  // Event handlers
  void
  StartSending (uint64_t iteration);

  void
  StopSending (uint64_t iteration);

  void
  SendFlit (uint64_t iteration);

  /** the period of the CTG */
  Time m_period;

  /** how many times the CTG is iterated, with the specified period */
  uint64_t m_iterations;

  /**
   * Which is the first iteration of the CTG that still that still needs to receive data.
   * For example, if iterations 0 and 1 of the CTG are in progress, than all the packets arriving at this node are for iteration 0.
   * Only after iteration 0 is finished, the packets received are for iteration 1.
   *
   **/
  uint64_t m_firstRunningIteration;

  uint32_t           m_hSize;                   // The horizontal size of a 2D mesh (how many nodes can be put on a line). The vertical size of the 2D mesh is given by number of nodes
  NetDeviceContainer m_devices;                 // the net devices from the NoC network
  NodeContainer      m_nodes;                   // the nodes from the NoC network
  uint32_t           m_flitSize;                // The flit size (the head flit will use part of this size for the flit header)
  uint16_t           m_numberOfFlits;           // How many flits a packet will have
  vector<uint16_t>   m_currentFlitIndex;        // the index of the flit to be injected ( [0, m_numberOfFlits - 1] )
  vector<Ptr<NocPacket> > m_currentHeadFlit;    // the current head flit
  uint32_t           m_maxBytes;                // Limit total number of bytes sent
  vector<uint32_t>   m_totBytes;                // Total bytes sent so far
  uint32_t           m_maxFlits;                // the maximum number of flits which may be injected (zero means no limit)
  vector<uint32_t>   m_totFlits;                // Total flits injected so far
  uint32_t           m_warmupCycles;            // During warmup cycles, no statistics are collected
  vector<EventId>    m_startEvent;              // Event id for next start event
  vector<EventId>    m_sendEvent;               // Event id of pending send flit event
  vector<bool>       m_injectionStarted;        // marks if this is the first time a flit is scheduled to be injected
  
  /** keeps all the tasks that are assigned to the IP core associated with this ns-3 application */
  list<TaskData> m_taskList;

  /**
   * Checks if the task list contains the specified task
   *
   * \param taskId the ID of the task to be searched
   *
   * \return true if the task was found, false otherwise
   *
   **/
  bool TaskListContainsTask (string taskId);

  /** the execution time of all tasks from the task list. This is computed when the task list is set */
  Time m_totalExecTime;

  /** keeps all the remote tasks that send data to this NoC node */
  list<DependentTaskData> m_taskSenderList;

  /** the total amount of data to be received at this node (in bits) */
  double m_totalData;

  /** keeps the amount of data currently received by this node (in bits). It obviously cannot exceed m_totalData */
  vector<double> m_receivedData;

  /** keeps all the local tasks that send data to tasks from remote NoC nodes */
  list<DependentTaskData> m_taskDestinationList;

  /** marks the element from the m_taskDestinationList list that is currently active for flit injection */
  vector<uint32_t> m_currentDestinationIndex;

  /**
   * Retrieves the item from the task destination list, located at the specified index
   *
   * \param the list index (must be >= 0 and < the size of the list)
   *
   * \return the DependentTaskData from the list
   */
  DependentTaskData GetDestinationDependentTaskData (uint32_t index);

  /** the total number of bytes sent to the current destination task */
  vector<uint32_t> m_totalTaskBytes;

  /**
   * Allows tracing injected packets into the network.
   * A packet is identified by its head flit.
   */
  TracedCallback<Ptr<const Packet> > m_packetInjectedTrace;

  /**
   * Traces the injected flits
   */
  TracedCallback<Ptr<const Packet> > m_flitInjectedTrace;

  /**
   * Traces the flits, when they are received at their destination
   */
  TracedCallback<Ptr<const Packet> > m_flitReceivedTrace;

  Time
  GetGlobalClock () const;

  void
  FlitReceivedCallback (std::string path, Ptr<const Packet> packet);

  void
  ScheduleNextTx (uint64_t iteration);


  /**
   * For each IP core from the CTG, the flit injection is just started with this method.
   * ScheduleNextTx (...) is used to continue the flit injection.
   *
   * \param iteration the CTG iteration number (0 is the first iteration)
   */
  void
  ScheduleStartEvent (uint64_t iteration);

};

} // namespace ns3

#endif
