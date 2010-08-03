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

#ifndef __noc_application_h__
#define __noc_application_h__

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/noc-packet.h"
#include "ns3/traffic-pattern.h"

namespace ns3 {

class Address;
class Socket;

/**
 *
 * \brief Network on Chip (NoC) application
 */
class NocApplication : public Application
{
public:

  enum TrafficPattern
    {
      /**
       * Dummy traffic pattern (the destination must be specified by the user)
       */
      DESTINATION_SPECIFIED,

      /**
       * Uses a NS-3 uniform random algorithm to determine the destination
       */
      UNIFORM_RANDOM,

      /**
       * Transposes the bits of the source to determine the destination
       * (the second half part of the bits are put in front of the first half).
       */
      BIT_MATRIX_TRANSPOSE,

      /**
       * Complements the bits of the source to determine the destination
       * (0 becomes 1 and 1 becomes 0).
       */
      BIT_COMPLEMENT,

      /**
       * Reverses the bits of the source to determine the destination
       * (the last bit becomes the first and so on).
       */
      BIT_REVERSE
    };

  static std::string TrafficPatternToString (TrafficPattern t);

  /**
   * \param a string which names the desired traffic pattern
   *
   * \return the traffic pattern (the default traffic pattern is UNIFORM_RANDOM)
   */
  static TrafficPattern TrafficPatternFromString (std::string t);

  static TypeId GetTypeId (void);

  NocApplication ();

  virtual ~NocApplication();

  void SetMaxBytes(uint32_t maxBytes);

  void SetNetDeviceContainer(NetDeviceContainer devices);

  void SetNodeContainer(NodeContainer nodes);

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  //helpers
  void CancelEvents ();

  // Event handlers
  void StartSending();
  void StopSending();
  void SendPacket();

  bool            m_connected;          // True if connected
  uint32_t        m_hSize;              // The horizontal size of a 2D mesh (how many nodes can be put on a line). The vertical size of the 2D mesh is given by number of nodes
  NetDeviceContainer m_devices;         // the net devices from the NoC network
  NodeContainer   m_nodes;              // the nodes from the NoC network
  DataRate        m_dataRate;           // Rate that data is generated
  uint32_t        m_pktSize;            // Size of data packets (head packets have the same size + the size of the header)
  uint16_t        m_numberOfPackets;    // How many packets a message will have
  uint16_t        m_currentPacketIndex; // the index of the packet to be injected ( [0, m_numberOfPackets - 1] )
  Ptr<NocPacket>  m_currentHeadPacket;  // the current head packet
  uint32_t        m_residualBits;       // Number of generated, but not sent, bits
  Time            m_lastStartTime;      // Time last packet sent
  uint32_t        m_maxBytes;           // Limit total number of bytes sent
  uint32_t        m_totBytes;           // Total bytes sent so far
  EventId         m_startEvent;         // Event id for next start event
  EventId         m_sendEvent;          // Event id of pending send packet event
  bool            m_sending;            // True if currently in sending state
  TracedCallback<Ptr<const Packet> > m_txTrace;
  ns3::TrafficPattern    m_trafficPattern;
  uint32_t           m_uniformDestinationX;     // the X coordinate of the last destination node (generated in an uniform random manner)
  uint32_t           m_uniformDestinationY;     // the Y coordinate of the last destination node (generated in an uniform random manner)
  
  /**
   * Allows tracing injected messages into the network.
   * A message is identified by its head packet.
   */
  TracedCallback<Ptr<const Packet> > m_messageTrace;

  /**
   * The traffic pattern which will be used by this application
   */
  TrafficPattern m_trafficPatternEnum;

  /**
   * The ID of the destination node. Note that this must be used only when the traffic
   * pattern is DESTINATION_SPECIFIED
   */
  uint32_t m_destinationNodeId;

private:

  void ScheduleNextTx();

  void ScheduleStartEvent();

  void Ignore(Ptr<Socket>);

};

} // namespace ns3

#endif

