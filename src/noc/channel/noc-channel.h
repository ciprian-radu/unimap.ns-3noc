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
#include "ns3/noc-packet.h"
#include "ns3/noc-net-device.h"
#include <map>

using namespace std;

namespace ns3
{

  class NocNetDevice;

  /**
   * \ingroup channel
   * \brief A channel for a Network on Chip (NoC)
   * \detail The NocChannel connects exactly two NocNetDevices.
   *         It is bidirectional and it allows the two net devices to
   *         communicate in half-duplex ("walkie-talkie" style) or full-duplex (telephone style) mode.
   *         By default the full-duplex mode is used.
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
    GetTypeId ();

    NocChannel ();

    virtual uint32_t
    Add (Ptr<NocNetDevice> device);

    // inherited from ns3::Channel
    virtual uint32_t
    GetNDevices () const;

    virtual Ptr<NetDevice>
    GetDevice (uint32_t i) const;

    /**
     * Get the assigned data rate of the channel
     *
     * \return the DataRate to be used by net device transmitters.
     */
    virtual DataRate
    GetDataRate ();

    /**
     * Get the assigned speed-of-light delay of the channel
     *
     * \return the delay used by the channel.
     */
    virtual Time
    GetDelay ();

    /**
     * \see WireState
     *
     * \param device the net device from the side of the channel where the state is checked (relevant only in full-duplex mode)
     *
     * \return the state of the channel
     */
    virtual WireState
    GetState (Ptr<NocNetDevice> device);

    /**
     * \brief Indicates if the channel is busy. The channel will only
     * accept new packets for transmission if it is not busy.
     *
     * \param device the net device from the side of the channel where the state is checked (relevant only in full-duplex mode)
     *
     * \return true if the channel is busy, false if it is free
     */
    virtual bool
    IsBusy (Ptr<NocNetDevice> device);

    /**
     * \brief Start transmitting a packet over the channel
     *
     * If the srcId belongs to a net device that is connected to the
     * channel, packet transmission begins, and the channel becomes busy
     * until the packet has completely reached all destinations.
     *
     * \param originalNetDevice the net device which generated this transmission
     *
     * \param p A reference to the packet that will be transmitted over
     * the channel
     *
     * \param srcId The device Id of the net device that wants to
     * transmit on the channel
     *
     * \return True if the channel is not busy
     */
    virtual bool
    TransmitStart (Ptr<NocNetDevice> originalNetDevice, Ptr<Packet> p, uint32_t srcId);

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
    virtual bool
    Send (Mac48Address to, Mac48Address from);

    /**
     * \brief Ends transmitting a packet over the channel, by actually passing it to the Receive method of the packet
     *
     * \see Send
     *
     * \param srcNocNetDevice the NoC net device that sent the packet
     *
     * \param to the address where the packet is to be sent
     *
     * \param destNocNetDevice the NoC net device that must receive the packet
     *
     * \param from the address from where the packet is sent
     *
     */
    virtual void
    TransmitEnd (Ptr<NocNetDevice> srcNocNetDevice, Mac48Address to,
        Ptr<NocNetDevice> destNocNetDevice, Mac48Address from);

private:

    /**
     * Uses ORION to measure this channel's dynamic power, consumed for sending the specified flit.
     *
     * \param flit the flit
     *
     * \return the dynamic power, in Watt
     */
    virtual double
    GetDynamicPower (Ptr<Packet> flit);

    /**
     * Uses ORION to measure this channel's leakage power.
     *
     * \return the dynamic power, in Watt
     */
    virtual double
    GetLeakagePower (Ptr<Packet> flit);

    /**
     * Power is measured for the previous clock cycle.
     * This method is invoked when methods GetDynamicPower () and GetLeakagePower () are called,
     * with the purpose to measure power for the last clock too.
     * The method has effect only if there are flits for which power wasn't measured.
     */
    void
    MeasurePowerForLastClock ();

public:

    /**
     * Uses ORION to get the dynamic power consumed by this channel.
     *
     * \return the dynamic power, in Watt
     */
    virtual double
    GetDynamicPower ();

    /**
     * Uses ORION to get the leakage power consumed by this channel.
     *
     * \return the dynamic power, in Watt
     */
    virtual double
    GetLeakagePower ();

    /**
     * \see GetDynamicPower
     * \see GetLeakagePower
     *
     * \return the dynamic + leakage power, in Watt
     */
    virtual double
    GetTotalPower ();

    /**
     * Uses ORION to measure this channel's area.
     *
     * \return the area, in um^2
     */
    virtual double
    GetArea ();

  private:

    /**
     * By default this field is true. If it is false, than the channel will work in half-duplex mode.
     */
    bool m_fullDuplex;

    /**
     * The two net devices connected at this channel.
     */
    vector<Ptr<NocNetDevice> > m_devices;

    /**
     * The assigned data rate of the channel
     */
    DataRate m_bps;

    /**
     * The assigned speed-of-light delay of the channel
     */
    Time m_delay;

    /**
     * The length of this wire, in um (micro meters)
     */
    double m_length;

    /**
     * Current state of each physical link of the channel.
     * There are 2 physical links in full-duplex mode and only one in half-duplex.
     */
    vector<WireState> m_state;

    /**
     * The Packet that is currently being transmitted on the channel (or the last
     * packet to have been transmitted on the channel if the channel is
     * free). Note that each physical link of the channel may transmit a packet.
     */
    vector<Ptr<Packet> > m_currentPkt;

    /**
     * how many times power consumption was measured
     */
    uint64_t m_powerCounter;

    /**
     * the last clock when power consumption was measured
     */
    uint64_t m_lastClock;

    /**
     * the flits from the previous clock cycle, for which power will have to be measured
     */
    vector<Ptr<Packet> > m_flitsFromLastClock;

    /**
     * the number of transmitted flits
     */
    uint64_t m_trasmittedFlits;

    double m_dynamicPower;

    double m_leakagePower;

    /**
     * The net device which tries to send a packet through this channel.
     * Typically, a packet stays in a net device and is sent via another
     * net device of the same node. Note that each physical link of the channel
     * has a net device which can send a packet (in full-duplex mode, both net
     * devices may send a packet at the same moment of time).
     */
    vector<map<uint32_t, Ptr<NocNetDevice> > > m_packetOriginalDevice;

    /**
     * The current destination net device (one for each physical channel)
     */
    vector<Ptr<NocNetDevice> > m_currentDestDevice;

  };

} // namespace ns3

#endif /* NOCCHANNEL_H_ */
