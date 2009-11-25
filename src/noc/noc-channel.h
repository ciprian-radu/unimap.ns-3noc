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

#ifndef NOCCHANNEL_H_
#define NOCCHANNEL_H_

#include "ns3/channel.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include <vector>
#include "noc-packet.h"
#include "noc-net-device.h"

namespace ns3
{

  class NocNetDevice;

  /**
   * \ingroup channel
   * \brief A channel for a Network on Chip (NoC)
   */
  class NocChannel : public Channel
  {
  public:

    /**
     * Current state of the channel
     */
    enum WireState
      {
        IDLE,          /**< Channel is IDLE, no packet is being transmitted */
        TRANSMITTING,  /**< Channel is BUSY, a packet is being written by a net device */
        PROPAGATING    /**< Channel is BUSY, packet is propagating to all attached net devices */
      };

    static TypeId
    GetTypeId(void);

    NocChannel();

    uint32_t
    Add(Ptr<NocNetDevice> device);

    // inherited from ns3::Channel
    virtual uint32_t
    GetNDevices(void) const;

    virtual Ptr<NetDevice>
    GetDevice(uint32_t i) const;

    /**
     * Get the assigned data rate of the channel
     *
     * \return the DataRate to be used by net device transmitters.
     */
    DataRate GetDataRate ();

    /**
     * Get the assigned speed-of-light delay of the channel
     *
     * \return the delay used by the channel.
     */
    Time GetDelay ();

    /**
     * \see WireState
     *
     * \return the state of the channel
     */
    WireState GetState ();

    /**
     * \brief Indicates if the channel is busy. The channel will only
     * accept new packets for transmission if it is not busy.
     *
     * \return true if the channel is busy, false if it is free
     */
    bool IsBusy ();

    /**
     * \brief Start transmitting a packet over the channel
     *
     * If the srcId belongs to a net device that is connected to the
     * channel, packet transmission begins, and the channel becomes busy
     * until the packet has completely reached all destinations.
     *
     * \param p A reference to the packet that will be transmitted over
     * the channel
     *
     * \param srcId The device Id of the net device that wants to
     * transmit on the channel
     *
     * \return True if the channel is not busy
     */
    bool TransmitStart (Ptr<Packet> p, uint32_t srcId);

    /**
     * \brief Transmits the packet. Please note that prior to calling this method, TransmitStart method must be called.
     *
     * \see TransmitStart
     *
     * \param to the address where the packet is to be sent
     *
     * \param from the address from where the packet is sent
     *
     */
    bool
    Send(Mac48Address to, Mac48Address from);

    /**
     * \brief Ends transmitting a packet over the channel, by actually passing it to the Receive method of the packet
     *
     * \see Send
     *
     * \param to the address where the packet is to be sent
     *
     * \param destNocNetDevice the NoC net device that must receive the packet
     *
     * \param from the address from where the packet is sent
     *
     */
    void TransmitEnd (Mac48Address to, Ptr<NocNetDevice> destNocNetDevice, Mac48Address from);

  private:

    std::vector<Ptr<NocNetDevice> > m_devices;

    /**
     * The assigned data rate of the channel
     */
    DataRate m_bps;

    /**
     * The assigned speed-of-light delay of the channel
     */
    Time m_delay;

    /**
     * Current state of the channel
     */
    WireState m_state;

    /**
     * The Packet that is currently being transmitted on the channel (or the last
     * packet to have been transmitted on the channel if the channel is
     * free)
     */
    Ptr<Packet> m_currentPkt;

    /**
     * Device Id of the source that is currently transmitting on the
     * channel (or last source to have transmitted a packet on the
     * channel, if the channel is currently not busy)
     */
    uint32_t m_currentSrc;

  };

} // namespace ns3

#endif /* NOCCHANNEL_H_ */
