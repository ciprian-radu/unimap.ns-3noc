/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
#ifndef NOC_SYNC_APPLICATION_HELPER_H
#define NOC_SYNC_APPLICATION_HELPER_H

#include <stdint.h>
#include <string>
#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/object-vector.h"
#include "ns3/noc-value.h"

namespace ns3
{

  /**
   * \brief A helper to make it easier to instantiate an ns3::NocSyncApplication
   * on a set of nodes.
   */
  class NocSyncApplicationHelper
  {
  public:
    /**
     * Create a NocSyncApplicationHelper to make it easier to work with NoC applications
     *
     * \param nodes the nodes from the NoC network
     *
     * \param devices the net devices from the NoC network
     *
     * \param hSize the horizontal size of the 2D mesh
     *
     */
    NocSyncApplicationHelper(NodeContainer nodes, NetDeviceContainer devices, uint32_t hSize);

    /**
     * Create a NocSyncApplicationHelper to make it easier to work with NoC applications
     *
     * \param nodes the nodes from the NoC network
     *
     * \param devices the net devices from the NoC network
     *
     * \param hSize the horizontal size of the 3D mesh
     *
     * \parm vSize the vertical size of the 3D mesh
     *
     */
    NocSyncApplicationHelper(NodeContainer nodes, NetDeviceContainer devices, uint32_t hSize, uint32_t vSize);

    /**
     * Create a NocSyncApplicationHelper to make it easier to work with NoC applications
     *
     * \param nodes the nodes from the NoC network
     *
     * \param devices the net devices from the NoC network
     *
     * \param size the size of every nD mesh dimension
     *
     */
    NocSyncApplicationHelper (NodeContainer nodes, NetDeviceContainer devices, std::vector<Ptr<NocValue> > size);

    /**
     * Helper function used to set the underlying application attributes.
     *
     * \param name the name of the application attribute to set
     * \param value the value of the application attribute to set
     */
    void
    SetAttribute(std::string name, const AttributeValue &value);

    /**
     * Install an ns3::OnOffApplication on each node of the input container
     * configured with all the attributes set with SetAttribute.
     *
     * \param c NodeContainer of the set of nodes on which an OnOffApplication
     * will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer
    Install(NodeContainer c) const;

    /**
     * Install an ns3::OnOffApplication on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an OnOffApplication will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer
    Install(Ptr<Node> node) const;

    /**
     * Install an ns3::OnOffApplication on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param nodeName The node on which an OnOffApplication will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer
    Install(std::string nodeName) const;

  private:
    /**
     * \internal
     * Install an ns3::OnOffApplication on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an OnOffApplication will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application>
    InstallPriv(Ptr<Node> node) const;
    std::string m_protocol;
    Address m_remote;
    ObjectFactory m_factory;
    NetDeviceContainer m_devices;
    NodeContainer m_nodes;
    std::vector<Ptr<NocValue> > m_size;
  };

} // namespace ns3

#endif /* NOC_SYNC_APPLICATION_HELPER_H */
