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
#include "noc-ctg-application-helper.h"
#include "ns3/noc-ctg-application.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/net-device-container.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE ("NocCtgApplicationHelper");

namespace ns3
{

  NocCtgApplicationHelper::NocCtgApplicationHelper (string mappingXmlFilePath, uint64_t iterations, uint64_t numberOfFlits,
      uint64_t simulationCycles, NodeContainer nodes, NetDeviceContainer devices, uint32_t hSize)
  {
    m_mappingXmlFilePath = mappingXmlFilePath;
    m_iterations = iterations;
    m_numberOfFlits = numberOfFlits;
    m_simulationCycles = simulationCycles;
    m_nodes = nodes;
    m_devices = devices;
    m_factory.SetTypeId ("ns3::NocCtgApplication");
    m_factory.Set ("HSize", UintegerValue (hSize));
    m_factory.Set ("Iterations", UintegerValue (m_iterations));
    m_factory.Set ("NumberOfFlits", UintegerValue (m_numberOfFlits));
  }

  void
  NocCtgApplicationHelper::SetAttribute (std::string name, const AttributeValue &value)
  {
    m_factory.Set (name, value);
  }

  list<NocCtgApplicationHelper::CoreData>
  NocCtgApplicationHelper::GetCoreDataList ()
  {
    return m_coreDataList;
  }

  apcgType::core_type
  NocCtgApplicationHelper::FindCoreInApcg (apcgType theApcgType, string coreId)
  {
    apcgType::core_type theApcgCoreType ("", "");
    for (apcgType::core_iterator i (theApcgType.core ().begin ()); i != theApcgType.core ().end (); i++)
      {
        if ((*i).uid () == coreId)
          {
            theApcgCoreType = *i;
            break;
          }
      }
    return theApcgCoreType;
  }

  string
  NocCtgApplicationHelper::FindNodeIdForTask (apcgType theApcgType, mappingType theMappingType, string taskId)
  {
    NS_LOG_FUNCTION_NOARGS ();

    string nodeId = "";

    for (apcgType::core_iterator i (theApcgType.core ().begin ()); i != theApcgType.core ().end (); i++)
      {
        apcgType::core_type theCoreType = *i;
        for (apcgType::core_type::task_const_iterator j (i->task ().begin ()); j != i->task ().end (); j++)
          {
            apcgType::core_type::task_type theApcgTaskType = *j;
            if (taskId == theApcgTaskType.id ())
              {
                for (mappingType::map_const_iterator k (theMappingType.map ().begin ()); k != theMappingType.map ().end (); k++)
                  {
                    mapType theMapType = *k;

                    if (theMapType.apcg () == theApcgType.id() && theMapType.core ().get () == theCoreType.uid ())
                      {
                        nodeId = theMapType.node ();
                        goto done;
                      }
                  }
              }
          }
      }

    done:

    if ("" == nodeId)
      {
        NS_LOG_ERROR ("Couldn't find the node of task with ID " << taskId);
      }
    else
      {
        NS_LOG_DEBUG ("Task with ID " << taskId << " belongs to node with ID " << nodeId);
      }

    return nodeId;
  }

  template<class T>
  bool
  NocCtgApplicationHelper::FromString (T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
  {
    std::istringstream iss(s);
    return !(iss >> f >> t).fail();
  }

  void
  NocCtgApplicationHelper::Initialize()
  {
    try
      {
        NS_LOG_INFO ("Initializing a CTG application for XML mapping " << m_mappingXmlFilePath);

        // parse the mapping XML
        auto_ptr<mappingType> theMappingType (research::noc::application_mapping::unified_framework::schema::mapping::mapping (
            m_mappingXmlFilePath, flags::dont_validate));

        for (mappingType::map_const_iterator i (theMappingType->map ().begin ()); i != theMappingType->map ().end (); i++)
          {
            mapType theMapType = *i;
            NS_LOG_INFO ("\t node: " << theMapType.node ());
            NS_LOG_INFO ("\t core: " << theMapType.core ());

            string ctgFilePath = m_mappingXmlFilePath.substr (0, m_mappingXmlFilePath.find_last_of (FILE_SEPARATOR));
            ctgFilePath = ctgFilePath.substr (0, ctgFilePath.find_last_of (FILE_SEPARATOR))
                + FILE_SEPARATOR + "ctg-" + theMapType.apcg ().substr (0, theMapType.apcg ().find_last_of ("_"));
            string apcgXmlFilePath = ctgFilePath + FILE_SEPARATOR + "apcg-" + theMapType.apcg () + ".xml";
            NS_LOG_LOGIC ("APCG XML file path is " << apcgXmlFilePath);

            // parse the APCG XML
            auto_ptr<apcgType> theApcgType (research::noc::application_mapping::unified_framework::schema::apcg::apcg (
                apcgXmlFilePath, flags::dont_validate));

            NS_ASSERT_MSG (theMapType.apcg () == theApcgType->id (),
                "Mapping XML says the APCG ID is " << theMapType.apcg ()
                << " but, APCG XML says the APCG ID is " << theApcgType->id () << "!");

            string ctgXmlFilePath = ctgFilePath + FILE_SEPARATOR + "ctg-" + theApcgType->ctg () + ".xml";
            NS_LOG_LOGIC ("CTG XML file path is " << ctgXmlFilePath);

            // parse the CTG XML
            auto_ptr<ctgType> theCtgType (research::noc::application_mapping::unified_framework::schema::ctg::ctg (ctgXmlFilePath,
                flags::dont_validate));

            NS_ASSERT_MSG (theApcgType->ctg () == theCtgType->id (),
                "APCG XML says the CTG ID is " << theApcgType->ctg ()
                << " but, CTG XML says the CTG ID is " << theCtgType->id () << "!");

            // extract all required information from all XMLs
            string coresFilePath = ctgFilePath.substr (0, ctgFilePath.find_last_of (FILE_SEPARATOR) + 1) + "cores";

            string tasksFilePath = ctgFilePath + FILE_SEPARATOR + "tasks";

            // Note that I am dereferencing theApcgType. If I am working with the (auto) pointer,
            // at the second iteration theApcgType becomes NULL and I don't know why...
            apcgType::core_type theApcgCoreType = FindCoreInApcg (*theApcgType, theMapType.core ().get ());
            NS_ASSERT_MSG (theApcgCoreType.uid () != "",
                "Error: couldn't find core with ID " << theMapType.core () << " in the APCG XML!");

            string coreXmlFilePath = coresFilePath + FILE_SEPARATOR + "core-" + theApcgCoreType.id () + ".xml";
            NS_LOG_LOGIC ("IP core XML file path is " << coreXmlFilePath);

            auto_ptr<research::noc::application_mapping::unified_framework::schema::core::coreType> theCoreType (
                research::noc::application_mapping::unified_framework::schema::core::core (coreXmlFilePath,
                    flags::dont_validate));
            NS_ASSERT_MSG (theApcgCoreType.id () == theCoreType->ID (), "APCG XML says the core ID is " << theApcgCoreType.id ()
                << " but, core XML says the ID is " << theCoreType->ID () << "!");
            NS_LOG_INFO ("\t (ID: " << theCoreType->ID ()
                << " name: " << theCoreType->name () << " frequency: "
                << theCoreType->frequency () << " (Hz) height: "
                << theCoreType->height () << " (m) width: "
                << theCoreType->width () << " (m) idle power: "
                << theCoreType->idlePower () << " (W) )");

            CoreData coreData;
            coreData.m_uid = theApcgCoreType.uid ();
            coreData.m_id = theCoreType->ID ();
            coreData.m_apcgId = theApcgType->id ();
            coreData.m_area = theCoreType->height ().get () * theCoreType->width ().get ();
            coreData.m_idlePower = theCoreType->idlePower ().get ();

            list<NocCtgApplication::TaskData> taskList;
            list<NocCtgApplication::DependentTaskData> remoteTaskList;
            list<NocCtgApplication::DependentTaskData> localTakList;

            for (apcgType::core_type::task_const_iterator i (theApcgCoreType.task ().begin ()); i
                != theApcgCoreType.task ().end (); i++)
              {
                NocCtgApplication::TaskData* taskData;

                apcgType::core_type::task_type theApcgTaskType = *i;
                NS_LOG_INFO ("\t\t task: " << theApcgTaskType.id ());

                string taskXmlFilePath = tasksFilePath + FILE_SEPARATOR + "task-" + theApcgTaskType.id () + ".xml";
                NS_LOG_LOGIC ("Task XML file path is " << taskXmlFilePath);

                auto_ptr<research::noc::application_mapping::unified_framework::schema::task::taskType> theTaskType (
                    research::noc::application_mapping::unified_framework::schema::task::task (taskXmlFilePath,
                        flags::dont_validate));
                NS_ASSERT_MSG (theApcgTaskType.id () == theTaskType->ID (), "APCG XML says the task ID is "
                    << theApcgTaskType.id () << " but, task XML says the ID is " << theTaskType->ID () << "!");
                NS_LOG_INFO ("\t\t (name: " << theTaskType->name () << " type: "
                    << theTaskType->type () << ")");

                // use the code commented below if you want to access some core information that is not in the APCG

//                for (research::noc::application_mapping::unified_framework::schema::core::coreType::task_const_iterator i (
//                    theCoreType->task ().begin ()); i != theCoreType->task ().end (); i++)
//                  {
//                    research::noc::application_mapping::unified_framework::schema::core::coreType::task_type theCoreTaskType =
//                        *i;
//                    if (theTaskType->type () == theCoreTaskType.type ())
//                      {
//                        NS_LOG_INFO ("\t\t execution time: " << theCoreTaskType.execTime ()
//                            << " (s) " << "power:" << theCoreTaskType.power ()
//                            << " (W) )");
//                        taskData = new NocCtgApplication::TaskData (theApcgTaskType.id (), Seconds (
//                            theCoreTaskType.execTime ().get ()));
//                        break;
//                      }
//                  }

                coreData.m_power[theApcgTaskType.id ()] = theApcgTaskType.power ().get ();
                coreData.m_execTime[theApcgTaskType.id ()] = theApcgTaskType.execTime ().get ();

                taskData
                    = new NocCtgApplication::TaskData (theApcgTaskType.id (), Seconds (theApcgTaskType.execTime ().get ()));
                taskList.insert (taskList.end (), *taskData);

                // build the task source and destination lists
                for (ctgType::communication_const_iterator i (theCtgType->communication ().begin ()); i
                    != theCtgType->communication ().end (); i++)
                  {
                    ctgType::communication_type theCommunicationType = *i;

                    if (theCommunicationType.source ().id () == taskData->GetId () || theCommunicationType.destination ().id ()
                        == taskData->GetId ())
                      {
                        NS_LOG_INFO ("\tsource task: " << theCommunicationType.source ().id ());
                        for (research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_const_iterator
                            i (theCommunicationType.source ().deadline ().begin ()); i
                            != theCommunicationType.source ().deadline ().end (); i++)
                          {
                            research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_type
                                deadline = *i;
                            NS_LOG_INFO ("\t\tdeadline: " << deadline.type () << " at " << deadline << " (ms)");
                          }NS_LOG_INFO ("\tdestination task: " << theCommunicationType.destination ().id ());

                        for (research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_const_iterator
                            i (theCommunicationType.destination ().deadline ().begin ()); i
                            != theCommunicationType.destination ().deadline ().end (); i++)
                          {
                            research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_type
                                deadline = *i;
                            NS_LOG_INFO ("\t\tdeadline: " << deadline.type () << " at " << deadline << " (ms)");
                          }

                        NS_LOG_INFO ("\tcommunication volume: " << theCommunicationType.volume () << " (bits)");

                        uint32_t sourceNodeId;
                        if (!FromString<uint32_t> (sourceNodeId, FindNodeIdForTask (*theApcgType, *theMappingType,
                            theCommunicationType.source ().id ()), std::dec))
                          {
                            NS_LOG_ERROR ("The source node ID is not a number");
                          }

                        uint32_t destinationNodeId;
                        if (!FromString<uint32_t> (destinationNodeId, FindNodeIdForTask (*theApcgType, *theMappingType,
                            theCommunicationType.destination ().id ()), std::dec))
                          {
                            NS_LOG_ERROR ("The destination node ID is not a number");
                          }

                        NocCtgApplication::DependentTaskData dependentTaskData = NocCtgApplication::DependentTaskData (
                            theCommunicationType.source ().id (), sourceNodeId, theCommunicationType.volume (),
                            theCommunicationType.destination ().id (), destinationNodeId);

                        if (theCommunicationType.destination ().id () == taskData->GetId ())
                          {
                            NS_LOG_DEBUG ("Inserting in the remote task list");

                            remoteTaskList.insert (remoteTaskList.end (), dependentTaskData);
                          }
                        if (theCommunicationType.source ().id () == taskData->GetId ())
                          {
                            NS_LOG_DEBUG ("Inserting in the local task list");

                            localTakList.insert (localTakList.end (), dependentTaskData);
                          }
                      }
                  }
              }

            m_coreDataList.insert (m_coreDataList.end (), coreData);

            SetAttribute ("Period", TimeValue (Seconds (theCtgType->period ().get ())));
            //          SetAttribute ("MaxFlits", UintegerValue (100));
            m_taskList = taskList;
            m_remoteTaskList = remoteTaskList;
            m_localTaskList = localTakList;

            uint32_t nodeId;
            if (!FromString<uint32_t> (nodeId, theMapType.node (), std::dec))
              {
                NS_LOG_ERROR ("The node ID " << theMapType.node () << " is not a number");
              }

            NS_ASSERT_MSG (m_nodes.GetN () > nodeId, "Cannot find NoC node "
                << nodeId << " because the NoC has only " << m_nodes.GetN () << " nodes. Check to see if this mapping is for this NoC topology!");

            ApplicationContainer apps = Install (m_nodes.Get (nodeId)); // source
            uint64_t startTime = 0;
            apps.Start (PicoSeconds (startTime));
            //          apps.Stop (PicoSeconds (10.0));
            //          // the application can also be limited by MaxPackets (the two ways of ending the application are equivalent)
            //          apps.Stop (PicoSeconds ((uint64_t) (m_simulationCycles * globalClock + startTime))); // stop = simulationCycles * globalClock + start

          }
      }
    catch (const xml_schema::exception& e)
      {
        NS_LOG_ERROR (e);
      }
  }

  Ptr<Application>
  NocCtgApplicationHelper::Install (Ptr<Node> node) const
  {
    Ptr<NocCtgApplication> app = m_factory.Create<NocCtgApplication> ();
    app->SetNetDeviceContainer (m_devices);
    app->SetNodeContainer (m_nodes);

    NS_LOG_DEBUG ("Installing an application on NoC node " << node->GetId ());

    node->AddApplication (app);

    // it is better to set the task lists after the application is
    // added to the node because the methods below use the node
    app->SetTaskList (m_taskList);
    app->SetRemoteTaskList (m_remoteTaskList);
    app->SetLocalTaskList (m_localTaskList);

    return app;
  }

} // namespace ns3
