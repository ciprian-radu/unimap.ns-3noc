/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 - 2011
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

#ifndef NOCTOPOLOGY_H_
#define NOCTOPOLOGY_H_

#include "ns3/noc-channel.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/noc-router.h"
#include "ns3/noc-routing-protocol.h"
#include "ns3/noc-switching-protocol.h"
#include "ns3/trace-helper.h"
#include "ns3/object-factory.h"

#include "../../../../NoC-XML/src/ro/ulbsibiu/acaps/noc/xml/link/link.hxx"
#include "../../../../NoC-XML/src/ro/ulbsibiu/acaps/noc/xml/node/node.hxx"
#include "../../../../NoC-XML/src/ro/ulbsibiu/acaps/noc/xml/topologyParameter/topology-parameter.hxx"

using namespace std;

using namespace research::noc::application_mapping::unified_framework::schema::link;
using namespace research::noc::application_mapping::unified_framework::schema::node;
using namespace research::noc::application_mapping::unified_framework::schema::topologyParameter;

using xml_schema::flags;

namespace ns3
{

  class NocChannel;

  /**
   * The topology for a Network-on-Chip.
   * Note that this is an abstract class.
   * It only describes the common characteristics of all NoC topologies.
   * Subclasses must specify how concrete topologies are installed using a set of NoC nodes.
   */
  class NocTopology : public Object
  {
  public:

    static const char FILE_SEPARATOR = '/';

    static TypeId
    GetTypeId();

    NocTopology ();

    virtual
    ~NocTopology ();

    void
    SetChannelAttribute (string n1, const AttributeValue &v1);

    /**
     * Set the type of input queue to create and associate to each
     * NocNetDevice created through NocTopology::Install.
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
    void
    SetInQueue (string type, string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (), string n2 = "",
        const AttributeValue &v2 = EmptyAttributeValue (), string n3 = "", const AttributeValue &v3 =
            EmptyAttributeValue (), string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

    /**
     * Set the type of output queue to create and associate to each
     * NocNetDevice created through NocTopology::Install.
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
    void
    SetOutQueue (string type, string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (), string n2 = "",
        const AttributeValue &v2 = EmptyAttributeValue (), string n3 = "", const AttributeValue &v3 =
            EmptyAttributeValue (), string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

    /**
     * Sets the router type. It must be a child of ns3::NocRouter
     *
     * \param type the router type
     *
     */
    void
    SetRouter (string type);

    /**
     * Sets a router attribute
     *
     * \param attributeName the name of the attribute
     * \param attributeValue the value of the attribute
     */
    void
    SetRouterAttribute (string attributeName, const AttributeValue &attributeValue);

    /**
     * Sets the routing protocol type. It must be a child of ns3::NocRoutingProtocol
     *
     * \param type the routing protocol type
     *
     */
    void
    SetRoutingProtocol (string type);

    /**
     * Sets a routing protocol attribute
     *
     * \param attributeName the name of the attribute
     * \param attributeValue the value of the attribute
     */
    void
    SetRoutingProtocolAttribute (string attributeName, const AttributeValue &attributeValue);

    /**
     * Sets the switching protocol type. It must be a child of ns3::NocSwitchingProtocol
     *
     * \param type the switching protocol type
     *
     */
    void
    SetSwitchingProtocol (string type);

    /**
     * Sets a switching protocol attribute
     *
     * \param attributeName the name of the attribute
     * \param attributeValue the value of the attribute
     */
    void
    SetSwitchingProtocolAttribute (string attributeName, const AttributeValue &attributeValue);

    /**
     * Uses ORION to get the dynamic power consumed by this NoC.
     *
     * \return the dynamic power, in Watt
     */
    double
    GetDynamicPower ();

    /**
     * Uses ORION to get the leakage power consumed by this NoC.
     *
     * \return the dynamic power, in Watt
     */
    double
    GetLeakagePower ();

    /**
     * \see GetDynamicPower
     * \see GetLeakagePower
     *
     * \return the dynamic + leakage power, in Watt
     */
    double
    GetTotalPower ();

    /**
     * Uses ORION to measure this NoC's area.
     *
     * \return the area, in um^2
     */
    double
    GetArea ();

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

    /**
     * Installs the topology on the given Network-on-Chip nodes
     *
     * \param nodes the NoC nodes
     */
    virtual NetDeviceContainer
    Install (NodeContainer nodes) = 0;

    /**
     * Computes the position of a destination node, relative to a source node.
     * The destination's relative position in each topological dimension is returned.
     *
     * \param sourceNodeId the source node
     * \param destinationNodeId the destination node
     *
     * \return an array with the destination's relative position in each dimension
     */
    virtual vector<uint8_t>
    GetDestinationRelativeDimensionalPosition (uint32_t sourceNodeId, uint32_t destinationNodeId) = 0;

    /**
     * Saves this topology using the NoC-XML interface: for each NocNode and NocChannel, an XML file is built.
     * The node XMLs are put in the nodes directory, and the link XMLs are put in the links directory.
     * Both nodes and links directories are subdirectories of a directory given as parameter
     *
     * \param nodes the NoC nodes for which this topology was installed
     * \param directoryPath the path to the directory where the topology will be saved
     *
     */
    virtual void
    SaveTopology (NodeContainer nodes, string directoryPath) = 0;

    Ptr<NocNetDevice>
    FindNetDeviceByAddress (Mac48Address address);

protected:

    NodeContainer m_nodes;

    NetDeviceContainer m_devices;

    ObjectFactory m_channelFactory;

    ObjectFactory m_inQueueFactory;

    ObjectFactory m_outQueueFactory;

    ObjectFactory m_routerFactory;

    ObjectFactory m_routingProtocolFactory;

    ObjectFactory m_switchingProtocolFactory;

  private:

    /*
     * \internal
     */
    static void
    AsciiTxEvent (Ptr<OutputStreamWrapper> stream, string path, Ptr<const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiRxEvent (Ptr<OutputStreamWrapper> stream, string path, Ptr<const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiEnqueueEvent (Ptr<OutputStreamWrapper> stream, string path, Ptr<const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiDequeueEvent (Ptr<OutputStreamWrapper> stream, string path, Ptr<const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiDropEvent (Ptr<OutputStreamWrapper> stream, string path, Ptr<const Packet> packet);

  };

} // namespace ns3

#endif /* NOCTOPOLOGY_H_ */
