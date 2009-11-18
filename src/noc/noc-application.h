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
#include "ns3/net-device-container.h"

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
  static TypeId GetTypeId (void);

  NocApplication ();

  virtual ~NocApplication();

  void SetMaxBytes(uint32_t maxBytes);

  void SetNetDeviceContainer(NetDeviceContainer devices);

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
  DataRate        m_dataRate;           // Rate that data is generated
  uint32_t        m_pktSize;            // Size of packets
  uint32_t        m_residualBits;       // Number of generated, but not sent, bits
  Time            m_lastStartTime;      // Time last packet sent
  uint32_t        m_maxBytes;           // Limit total number of bytes sent
  uint32_t        m_totBytes;           // Total bytes sent so far
  EventId         m_startEvent;         // Event id for next start event
  EventId         m_sendEvent;          // Event id of pending send packet event
  bool            m_sending;            // True if currently in sending state
  TypeId          m_tid;
  TracedCallback<Ptr<const Packet> > m_txTrace;
  
private:
  void ScheduleNextTx();
  void ScheduleStartEvent();
  void Ignore(Ptr<Socket>);
};

} // namespace ns3

#endif

