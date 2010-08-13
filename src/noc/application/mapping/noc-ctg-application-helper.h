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
#ifndef NOC_CTG_APPLICATION_HELPER_H
#define NOC_CTG_APPLICATION_HELPER_H

#include <stdint.h>
#include <string>
#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "noc-ctg-application.h"

using namespace std;

namespace ns3
{

  /**
   * \brief A helper to make it easier to instantiate an ns3::NocCtgApplication
   * on a set of nodes.
   */
  class NocCtgApplicationHelper
  {
  public:

    /**
     * Create a NocCtgApplicationHelper to make it easier to work with NoC applications
     *
     * \param nodes the nodes from the NoC network
     *
     * \param devices the net devices from the NoC network
     *
     * \param hSize the horizontal size of the 2D mesh
     *
     * \param taskList keeps all the tasks that are assigned to the IP core associated with this ns-3 application
     *
     * \param taskSenderList keeps all the remote tasks that send data to this NoC node
     *
     */
    NocCtgApplicationHelper(NodeContainer nodes, NetDeviceContainer devices,
        uint32_t hSize, list<NocCtgApplication::TaskData> taskList,
        list<NocCtgApplication::DependentTaskData> taskSenderList);

    /**
     * Helper function used to set the underlying application attributes.
     *
     * \param name the name of the application attribute to set
     * \param value the value of the application attribute to set
     */
    void
    SetAttribute(std::string name, const AttributeValue &value);

    /**
     * Install an application on each node of the input container
     * configured with all the attributes set with SetAttribute.
     *
     * \param c NodeContainer of the set of nodes on which an application
     * will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer
    Install(NodeContainer c) const;

    /**
     * Install an application on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an application will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer
    Install(Ptr<Node> node) const;

    /**
     * Install an application on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param nodeName The node on which an application will be installed.
     * \returns Container of Ptr to the applications installed.
     */
    ApplicationContainer
    Install(std::string nodeName) const;

  private:

    /**
     * \internal
     * Install an application on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an application will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application>
    InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory;

    NetDeviceContainer m_devices;

    NodeContainer m_nodes;

    /**
     * keeps all the tasks that are assigned to the IP core associated with this ns-3 application
     * (this is passed to NocCtgApplication)
     **/
    list<NocCtgApplication::TaskData> m_taskList;

    /**
     * keeps all the remote tasks that send data to this NoC node
     * (this is passed to NocCtgApplication)
     **/
    list<NocCtgApplication::DependentTaskData> m_taskSenderList;
  };

} // namespace ns3

#endif /* NOC_CTG_APPLICATION_HELPER_H */

