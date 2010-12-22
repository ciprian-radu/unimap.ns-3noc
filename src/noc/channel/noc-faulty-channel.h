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

#ifndef NOCFAULTYCHANNEL_H_
#define NOCFAULTYCHANNEL_H_

#include "ns3/channel.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include <vector>
#include "ns3/noc-packet.h"
#include "ns3/noc-net-device.h"

namespace ns3
{

  class NocNetDevice;

  /**
   * A NocChannel which can be faulty.
   * The faulty property is an ns-3 attribute.
   *
   * \see NocChannel
   */
  class NocFaultyChannel : public NocChannel
  {
  public:

    static TypeId
    GetTypeId ();

    NocFaultyChannel ();

    virtual uint32_t
    Add (Ptr<NocNetDevice> device);

    virtual uint32_t
    GetNDevices () const;

    virtual Ptr<NetDevice>
    GetDevice (uint32_t i) const;

    /**
     * \see WireState
     *
     * \return the state of the channel
     */
    virtual WireState
    GetState (Ptr<NocNetDevice> device);

    /**
     * \brief Indicates if the channel is busy. The channel will only
     * accept new packets for transmission if it is not busy.
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
     * whether or not the node is faulty
     */
    bool m_faulty;

  };

} // namespace ns3

#endif /* NOCFAULTYCHANNEL_H_ */
