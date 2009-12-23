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

#ifndef SLBROUTING_H_
#define SLBROUTING_H_

#include "ns3/noc-routing-protocol.h"
#include <vector>

namespace ns3
{

  /**
   * \brief Static Load Bound routing algorithm. Information about the channel load
   * of the neighboring nodes is used in a static fashion.
   *
   * \detail
   * http://www.informatik.uni-augsburg.de/de/lehrstuehle/sik/publikationen/finished_thesises/200709_schlingmann/200709_schlingmann.pdf
   */
  class SlbRouting : public NocRoutingProtocol
  {
  public:

    enum Direction {NONE, NORTH, EAST, SOUTH, WEST};

    static TypeId
    GetTypeId ();

    /**
     * Constructor
     *
     */
    SlbRouting ();

    virtual
    ~SlbRouting ();

    virtual bool
    RequestNewRoute (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination,
        Ptr<Packet> packet, RouteReplyCallback routeReply);

  private:

    /**
     * the chosen direction is progressive (leads to the target)
     */
    int progressiveWeight;

    /**
     * the output channel in the chosen direction is currently occupied
     */
    int remainingWeight;

    /**
     * the router in the chosen direction is loaded
     */
    int loadWeight;

    /**
     * Determines all the possible net devices that could route the packet
     *
     * \param source the net device through which the packet arrived
     * \param destination the destination node of the packet
     * \param packet the packet to be routed
     *
     * \return an array with all the possible net devices that could route the packet
     */
    std::vector<Ptr<NocNetDevice> >
    DoRoutingFunction (const Ptr<NocNetDevice> source,
        const Ptr<NocNode> destination, Ptr<Packet> packet);

    /**
     * Selects the net device which will be used for routing. This is done by evaluating all the possible devices.
     *
     * \param devices the devices from which the selection is made
     * \param source the net device through which the packet arrived
     * \param destination the destination node of the packet
     * \param packet the packet to be routed
     *
     * \return the selected net device
     */
    Ptr<NocNetDevice>
    DoSelectionFunction (std::vector<Ptr<NocNetDevice> > devices,
        const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet);

    /**
     * Evaluates the given net device to determine how profitable it is to route the packet through it.
     *
     * \param device the net device
     * \param packet the packet to be routed
     *
     * \return the result of the evaluation
     */
    int
    Evaluate (Ptr<NocNetDevice> device, Ptr<Packet> packet);

    /**
     * \param packet the packet to be routed
     * \param device the net device
     *
     * \return whether or not the direction is progressive
     */
    static bool
    IsProgressiveDirection (Ptr<Packet> packet, Ptr<NocNetDevice> device);

  };

} // namespace ns3

#endif /* SLBROUTING_H_ */
