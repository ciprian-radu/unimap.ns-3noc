/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010
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

#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/mapping/mapping.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/apcg/apcg.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/ctg/ctg.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/core/core.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/task/task.hxx"

using namespace ns3;

using namespace research::noc::application_mapping::unified_framework::schema::mapping;
using namespace research::noc::application_mapping::unified_framework::schema::apcg;
using namespace research::noc::application_mapping::unified_framework::schema::ctg;
using namespace research::noc::application_mapping::unified_framework::schema::core;
using namespace research::noc::application_mapping::unified_framework::schema::task;

using xml_schema::flags;

const char FILE_SEPARATOR = '/';

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
     * Simple data structure that keeps information regarding core power and area.
     */
    class CoreData
     {
       public:

         /** the unique ID of the core */
         string m_uid;

         /** the ID (type) of the core */
         string m_id;

         /** the ID of the APCG to which the core belongs */
         string m_apcgId;

         /** the power consumed by the core when idle, in Watts */
         double m_idlePower;

         /** how much power (in Watts) the core requires for executing a task assigned to it. (task ID, power/task) pairs are used. */
         map<string, double> m_power;

         /** how much time (in seconds) the core needs for executing a task assigned to it. (task ID, time/task) pairs are used. */
         map<string, double> m_execTime;

         /** the core area, in m^2 */
         double m_area;

     };

    /**
     * Create a NocCtgApplicationHelper to make it easier to work with NoC applications
     *
     * \param mappingXmlFilePaths the mapping XML file path
     *
     * \param iterations how many times the CTGs will be iterated
     *
     * \param flitSize the size of a flit, in bytes
     *
     * \param numberOfFlits the number of flits per packet
     *
     * \param simulationCycles how many simulation cycles the CTGs are allowed to run (this overrides the number of CTG iterations)
     *
     * \param nodes the nodes from the NoC network
     *
     * \param devices the net devices from the NoC network
     *
     * \param hSize the horizontal size of the 2D mesh
     *
     * \see Initialize ()
     *
     */
    NocCtgApplicationHelper (string mappingXmlFilePath, uint64_t iterations, uint64_t numberOfFlits,
        uint64_t simulationCycles, NodeContainer nodes, NetDeviceContainer devices, uint32_t hSize);

    /**
     * Reads the data from each mapping XML and installs the NoC CTG applications
     */
    void
    Initialize ();

    /**
     * Helper function used to set the underlying application attributes.
     *
     * \param name the name of the application attribute to set
     * \param value the value of the application attribute to set
     */
    void
    SetAttribute (std::string name, const AttributeValue &value);

    list<CoreData>
    GetCoreDataList ();

  private:

    apcgType::core_type
    FindCoreInApcg (apcgType theApcgType, string coreId);

    string
    FindNodeIdForTask (apcgType theApcgType, mappingType theMappingType, string taskId);

    template<class T>
    bool
    FromString (T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&));

    /**
     * \internal
     * Install an application on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an application will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application>
    Install (Ptr<Node> node) const;

    ObjectFactory m_factory;

    NetDeviceContainer m_devices;

    NodeContainer m_nodes;

    /** the mapping XML file path */
    string m_mappingXmlFilePath;

    /** how many times the CTGs will be iterated */
    uint64_t m_iterations;

    /** the number of flits per packet */
    uint64_t m_numberOfFlits;

    /** how many simulation cycles the CTGs are allowed to run (this overrides the number of CTG iterations) */
    uint64_t m_simulationCycles;

    /**
     * keeps all the tasks that are assigned to the IP core associated with this ns-3 application
     * (this is passed to NocCtgApplication)
     **/
    list<NocCtgApplication::TaskData> m_taskList;

    /**
     * keeps all the remote tasks that send data to this NoC node
     * (this is passed to NocCtgApplication)
     **/
    list<NocCtgApplication::DependentTaskData> m_remoteTaskList;

    /**
     * keeps all the local tasks that send data to tasks from remote NoC nodes
     * (this is passed to NocCtgApplication)
     **/
    list<NocCtgApplication::DependentTaskData> m_localTaskList;

    /**
     * keeps power and area information about all the core
     */
    list<CoreData> m_coreDataList;

  };

} // namespace ns3

#endif /* NOC_CTG_APPLICATION_HELPER_H */

