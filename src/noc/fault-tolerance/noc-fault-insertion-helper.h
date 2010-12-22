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

#ifndef NOCFAULTINSERTIONHELPER_H_
#define NOCFAULTINSERTIONHELPER_H_

#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/type-id.h"
#include "ns3/boolean.h"

namespace ns3
{

  /**
   * Helper class which can be used to ease the process of inserting
   * faults into the Network-on-Chip architecture.
   * Note that this class expects the "Faulty" attribute to belong to any
   * NoC component affected.
   */
  class NocFaultInsertionHelper : public Object
  {
  public:

    static TypeId
    GetTypeId ();

    /**
     * Constructor
     *
     * \param nodeContainer this node container is supposed to hold all the nodes of the NoC
     */
    NocFaultInsertionHelper (NodeContainer nodeContainer);

    virtual
    ~NocFaultInsertionHelper ();

    /**
     * Marks the specified node as faulty.
     * The node is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the node is already faulty, only a warning (log) message will be issued.
     *
     * \param nodeId the ID of the node
     */
    virtual void
    SetNodeAsFaulty (uint32_t nodeId);

    /**
     * Marks the specified node as faulty, after a specified time delay.
     * \see SetNodeAsFaulty (uint32_t nodeId)
     *
     * \param nodeId the ID of the node
     * \param delay after what time the node will be set as faulty
     */
    virtual void
    SetNodeAsFaultyAfterDelay (uint32_t nodeId, Time delay);

    /**
     * Marks the specified node as not faulty (i.e. working).
     * The node is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the node is already not faulty, only a warning (log) message will be issued.
     *
     * \param nodeId the ID of the node
     */
    virtual void
    SetNodeAsNotFaulty (uint32_t nodeId);

    /**
     * Marks the specified node as not faulty, after a specified time delay.
     * \see SetNodeAsNotFaulty (uint32_t nodeId)
     *
     * \param nodeId the ID of the node
     * \param delay after what time the node will be set as not faulty
     */
    virtual void
    SetNodeAsNotFaultyAfterDelay (uint32_t nodeId, Time delay);

    /**
     * Marks the router of the specified node as faulty.
     * The node is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the router of the node is already faulty, only a warning (log) message will be issued.
     *
     * \param nodeId the ID of the node that has the desired router
     */
    virtual void
    SetNodeRouterAsFaulty (uint32_t nodeId);

    /**
     * Marks the specified router as faulty, after a specified time delay.
     * \see SetNodeRouterAsFaulty (uint32_t nodeId)
     *
     * \param nodeId the ID of the node that has the desired router
     * \param delay after what time the router will be set as faulty
     */
    virtual void
    SetNodeRouterAsFaultyAfterDelay (uint32_t nodeId, Time delay);

    /**
     * Marks the router of the specified node as not faulty (i.e. working).
     * The node is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the router of the node is already not faulty, only a warning (log) message will be issued.
     *
     * \param nodeId the ID of the node that has the desired router
     */
    virtual void
    SetNodeRouterAsNotFaulty (uint32_t nodeId);

    /**
     * Marks the specified router as not faulty, after a specified time delay.
     * \see SetNodeRouterAsNotFaulty (uint32_t nodeId)
     *
     * \param nodeId the ID of the node that has the desired router
     * \param delay after what time the router will be set as not faulty
     */
    virtual void
    SetNodeRouterAsNotFaultyAfterDelay (uint32_t nodeId, Time delay);

    /**
     * Marks the specified net device as faulty.
     * The net device is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the net device is already faulty, only a warning (log) message will be issued.
     *
     * \param netDeviceAddress the address of the net device
     */
    virtual void
    SetNetDeviceAsFaulty (Address netDeviceAddress);

    /**
     * Marks the specified net device as faulty, after a specified time delay.
     * \see SetNetDeviceAsFaulty (Address netDeviceAddress)
     *
     * \param netDeviceAddress the address of the net device
     * \param delay after what time the net device will be set as faulty
     */
    virtual void
    SetNetDeviceAsFaultyAfterDelay (Address netDeviceAddress, Time delay);

    /**
     * Marks the specified net device as not faulty (i.e. working).
     * The net device is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the net device is already not faulty, only a warning (log) message will be issued.
     *
     * \param netDeviceAddress the address of the net device
     */
    virtual void
    SetNetDeviceAsNotFaulty (Address netDeviceAddress);

    /**
     * Marks the specified net device as not faulty, after a specified time delay.
     * \see SetNetDeviceAsNotFaulty (Address netDeviceAddress)
     *
     * \param netDeviceAddress the address of the net device
     * \param delay after what time the net device will be set as not faulty
     */
    virtual void
    SetNetDeviceAsNotFaultyAfterDelay (Address netDeviceAddress, Time delay);

    /**
     * Marks the specified channel as faulty.
     * The channel is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the channel is already faulty, only a warning (log) message will be issued.
     *
     * \param channelId the ID of the channel
     */
    virtual void
    SetChannelAsFaulty (uint32_t channelId);

    /**
     * Marks the specified channel as not faulty, after a specified time delay.
     * \see SetChannelAsFaulty (uint32_t channelId)
     *
     * \param channelId the ID of the channel
     * \param delay after what time the node will be set as faulty
     */
    virtual void
    SetChannelAsFaultyAfterDelay (uint32_t channelId, Time delay);

    /**
     * Marks the specified channel as not faulty (i.e. working).
     * The channel is searched in the node container of this object (i.e. the Noc).
     * If it is not found, the method will only issue a warning (log) message.
     * If the channel is already not faulty, only a warning (log) message will be issued.
     *
     * \param channelId the ID of the channel
     */
    virtual void
    SetChannelAsNotFaulty (uint32_t nodeId);

    /**
     * Marks the specified channel as not faulty, after a specified time delay.
     * \see SetChannelAsNotFaulty (uint32_t channelId)
     *
     * \param channelId the ID of the channel
     * \param delay after what time the node will be set as not faulty
     */
    virtual void
    SetChannelAsNotFaultyAfterDelay (uint32_t channelId, Time delay);

  private:

    static const std::string FAULTY_ATTRIBUTE;

    NodeContainer m_nodeContainer;

  };

} // namespace ns3

#endif /* NOCFAULTINSERTIONHELPER_H_ */
