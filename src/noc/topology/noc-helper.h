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

#ifndef NOCHELPER_H_
#define NOCHELPER_H_

#include "ns3/noc-channel.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/trace-helper.h"
#include "ns3/object-factory.h"

namespace ns3
{

  class NocChannel;

  class NocHelper : public Object
  {
  public:

    NocHelper ();

    void
    SetChannelAttribute (std::string n1, const AttributeValue &v1);

    /**
     * Set the type of input queue to create and associate to each
     * NocNetDevice created through NocHelper::Install.
     *
     * \param type the type of input queue
     * \param n1 the name of the attribute to set on the queue
     * \param v1 the value of the attribute to set on the queue
     * \param n2 the name of the attribute to set on the queue
     * \param v2 the value of the attribute to set on the queue
     * \param n3 the name of the attribute to set on the queue
     * \param v3 the value of the attribute to set on the queue
     * \param n4 the name of the attribute to set on the queue
     * \param v4 the value of the attribute to set on the queue
     *
     */
    void SetInQueue (std::string type,
                     std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                     std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                     std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                     std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

    /**
     * Set the type of output queue to create and associate to each
     * NocNetDevice created through NocHelper::Install.
     *
     * \param type the type of output queue
     * \param n1 the name of the attribute to set on the queue
     * \param v1 the value of the attribute to set on the queue
     * \param n2 the name of the attribute to set on the queue
     * \param v2 the value of the attribute to set on the queue
     * \param n3 the name of the attribute to set on the queue
     * \param v3 the value of the attribute to set on the queue
     * \param n4 the name of the attribute to set on the queue
     * \param v4 the value of the attribute to set on the queue
     *
     */
    void SetOutQueue (std::string type,
                     std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                     std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                     std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                     std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

    /**
     * \param stream output stream wrapper
     * \param nodeid the id of the node to generate ascii output for.
     * \param deviceid the id of the device to generate ascii output for.
     *
     * Enable ascii output on the specified deviceid within the
     * specified nodeid if it is of type ns3::NocNetDevice and dump
     * that to the specified stdc++ output stream.
     */
    static void
    EnableAscii (Ptr<OutputStreamWrapper> stream, uint32_t nodeid, uint32_t deviceid);

    /**
     * \param stream output stream wrapper
     * \param d device container
     *
     * Enable ascii output on each device which is of the
     * ns3::NocNetDevice type and which is located in the input
     * device container and dump that to the specified
     * stdc++ output stream.
     */
    static void
    EnableAscii (Ptr<OutputStreamWrapper> stream, NetDeviceContainer d);

    /**
     * \param stream output stream wrapper
     * \param n node container
     *
     * Enable ascii output on each device which is of the
     * ns3::NocNetDevice type and which is located in one
     * of the input node and dump that to the specified
     * stdc++ output stream.
     */
    static void
    EnableAscii (Ptr<OutputStreamWrapper> stream, NodeContainer n);

    /**
     * \param stream output stream wrapper
     *
     * Enable ascii output on each device which is of the
     * ns3::NocNetDevice type and dump that to the specified
     * stdc++ output stream.
     */
    static void
    EnableAsciiAll (Ptr<OutputStreamWrapper> stream);

    NetDeviceContainer
    Install (NodeContainer nodes, Ptr<NocChannel> channel);

    NetDeviceContainer
    Install (NodeContainer nodes);

    /**
     * Creates a typical 2-dimensional mesh network topology.
     *
     * \see ns3::FourWayRouter
     *
     * The router works with a Dimension Ordered Routing protocol
     *
     * \param nodes the NoC nodes for which the topology will be installed
     * \param hSize how many nodes will be put on each row of the mesh
     * \param routingProtocolFactory the routing protocol factory
     * \param switchingProtocolFactory the switching protocol factory
     *
     */
    NetDeviceContainer
    Install2DMesh (NodeContainer nodes, uint32_t hSize, ObjectFactory routerFactory,
        ObjectFactory routingProtocolFactory, ObjectFactory switchingProtocolFactory);

    /**
     * Creates a 2-dimensional mesh network topology Irvine architecture.
     *
     * \see ns3::IrvineRouter
     *
     * The Irvine router works with a Dimension Ordered Routing protocol
     *
     * \param nodes the NoC nodes for which the topology will be installed
     * \param hSize how many nodes will be put on each row of the mesh
     * \param routingProtocolFactory the routing protocol factory
     * \param switchingProtocolFactory the switching protocol factory
     *
     */
    NetDeviceContainer
    Install2DMeshIrvine (NodeContainer nodes, uint32_t hSize, ObjectFactory routerFactory,
        ObjectFactory routingProtocolFactory, ObjectFactory switchingProtocolFactory);

    Ptr<NocNetDevice>
    FindNetDeviceByAddress (Mac48Address address);

  private:

    static void
    AsciiTxEvent (Ptr<OutputStreamWrapper> stream, std::string path,
        Ptr<const Packet> packet);

    static void
    AsciiRxEvent (Ptr<OutputStreamWrapper> stream, std::string path,
        Ptr<const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiEnqueueEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<
        const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiDequeueEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<
        const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiDropEvent (Ptr<OutputStreamWrapper> stream, std::string path,
        Ptr<const Packet> packet);

    NetDeviceContainer m_devices;

    ObjectFactory m_channelFactory;

    ObjectFactory m_inQueueFactory;

    ObjectFactory m_outQueueFactory;
  };

} // namespace ns3

#endif /* NOCHELPER_H_ */
