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
#include <iostream>

#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/mapping/mapping.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/apcg/apcg.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/ctg/ctg.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/core/core.hxx"
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/task/task.hxx"

#include "ns3/assert.h"

using namespace std;

using namespace research::noc::application_mapping::unified_framework::schema::mapping;
using namespace research::noc::application_mapping::unified_framework::schema::apcg;
using namespace research::noc::application_mapping::unified_framework::schema::ctg;
using namespace research::noc::application_mapping::unified_framework::schema::core;
using namespace research::noc::application_mapping::unified_framework::schema::task;

using xml_schema::flags;

const char FILE_SEPARATOR = '/';

apcgType::core_type
findCoreInApcg(apcgType theApcgType, string coreId)
{
  apcgType::core_type theApcgCoreType("", "");
  for (apcgType::core_iterator i(theApcgType.core().begin()); i
      != theApcgType.core().end(); i++)
    {
      if ((*i).uid() == coreId)
        {
          theApcgCoreType = *i;
          break;
        }
    }
  return theApcgCoreType;
}

int
main(int argc, char* argv[])
{

  try
    {
      // parse the mapping XML

      string mappingXmlFilePath(
          "../Mapper/xml/e3s/auto-indust-mocsyn.tgff/ctg-0+1/mapping-0+1_1_0.xml");

      auto_ptr<mappingType>
          theMappingType(
              research::noc::application_mapping::unified_framework::schema::mapping::mapping(
                  mappingXmlFilePath, flags::dont_validate));

      cerr << "Printing the core to node mappings" << endl;

      for (mappingType::map_const_iterator i (theMappingType->map ().begin ()); i != theMappingType->map ().end (); i++)
        {
          mapType theMapType = *i;
          cerr << "core: " << theMapType.core () << endl;
          cerr << "node: " << theMapType.node () << endl;

          cerr << "Mapping ID: " << theMappingType->id () << " APCG ID: " << theMapType.apcg () << endl;

          //      for (mappingType::map_const_iterator i(theMappingType->map().begin()); i
          //          != theMappingType->map().end(); i++)
          //        {
          //          mapType theMapType = *i;
          //          cerr << "\t node: " << theMapType.node() << endl;
          //          cerr << "\t core: " << theMapType.core() << endl;
          //          cerr << endl;
          //        }

          string ctgFilePath = mappingXmlFilePath.substr (0, mappingXmlFilePath.find_last_of (FILE_SEPARATOR));
          //      cerr << "CTG file path: " << ctgFilePath << endl;

          string apcgXmlFilePath = ctgFilePath + FILE_SEPARATOR + "apcg-" + theMapType.apcg () + ".xml";
          //      cerr << "APCG XML file path: " << apcgXmlFilePath << endl;
          //      cerr << endl;

          // parse the APCG XML

          auto_ptr<apcgType> theApcgType (research::noc::application_mapping::unified_framework::schema::apcg::apcg (
              apcgXmlFilePath, flags::dont_validate));

          NS_ASSERT_MSG (theMapType.apcg () == theApcgType->id (), "Mapping XML says the APCG ID is " << theMapType.apcg ()
              << " but, APCG XML says the APCG ID is " << theApcgType->id() << "!");

          //      for (apcgType::core_const_iterator i(theApcgType->core().begin()); i
          //          != theApcgType->core().end(); i++)
          //        {
          //          apcgType::core_type theApcgCoreType = *i;
          //          cerr << "\t core: " << theApcgCoreType.id() << endl;
          //          for (apcgType::core_type::task_const_iterator i(
          //              theApcgCoreType.task().begin()); i != theApcgCoreType.task().end(); i++)
          //            {
          //              apcgType::core_type::task_type theTaskType = *i;
          //              cerr << "\t\t task: " << theTaskType << endl;
          //              cerr << endl;
          //            }
          //        }

          string ctgXmlFilePath = ctgFilePath + FILE_SEPARATOR + "ctg-" + theApcgType->ctg () + ".xml";
          //      cerr << "CTG XML file path: " << ctgXmlFilePath << endl;
          //      cerr << endl;

          // parse the CTG XML

          auto_ptr<ctgType> theCtgType (research::noc::application_mapping::unified_framework::schema::ctg::ctg (
              ctgXmlFilePath, flags::dont_validate));

          NS_ASSERT_MSG (theApcgType->ctg() == theCtgType->id(),
              "APCG XML says the CTG ID is " << theApcgType->ctg()
              << " but, CTG XML says the CTG ID is " << theCtgType->id() << "!");

          cerr << "CTG ID: " << theCtgType->id () << " CTG period: " << theCtgType->period () << " (ms)" << endl;

          // extract all required information from all XMLs

          string coresFilePath = ctgFilePath.substr (0, ctgFilePath.find_last_of (FILE_SEPARATOR) + 1) + "cores";

          string tasksFilePath = ctgFilePath + FILE_SEPARATOR + "tasks";

          string coreXmlFilePath = coresFilePath + FILE_SEPARATOR + "core-"
              + theMapType.core().get() + ".xml";

          auto_ptr<
              research::noc::application_mapping::unified_framework::schema::core::coreType>
              theCoreType(
                  research::noc::application_mapping::unified_framework::schema::core::core(
                      coreXmlFilePath, flags::dont_validate));
          NS_ASSERT_MSG (theMapType.core().get() == theCoreType->ID(), "APCG XML says the core ID is " << theMapType.core()
              << " but, core XML says the ID is " << theCoreType->ID() << "!");
          cerr << " (name: " << theCoreType->name() << " frequency: "
              << theCoreType->frequency() << " (Hz) height: "
              << theCoreType->height() << " (m) width: "
              << theCoreType->width() << " (m) idle power: "
              << theCoreType->idlePower() << " (W) )" << endl;

          // Note that I am dereferencing theApcgType. If I am working with the (auto) pointer,
          // at the second iteration theApcgType becomes NULL and I don't know why...
          apcgType::core_type theApcgCoreType = findCoreInApcg(*theApcgType,
              theMapType.core().get());
          NS_ASSERT_MSG (theApcgCoreType.uid() != "",
              "Error: couldn't find core with ID " << theMapType.core() << " in the APCG XML!");
          for (apcgType::core_type::task_const_iterator i (theApcgCoreType.task ().begin ()); i
              != theApcgCoreType.task ().end (); i++)
            {
              apcgType::core_type::task_type theApcgTaskType = *i;
              cerr << "task: " << theApcgTaskType.id ();

              string taskXmlFilePath = tasksFilePath + FILE_SEPARATOR + "task-" + theApcgTaskType.id () + ".xml";

              auto_ptr<research::noc::application_mapping::unified_framework::schema::task::taskType> theTaskType (
                  research::noc::application_mapping::unified_framework::schema::task::task (taskXmlFilePath,
                      flags::dont_validate));
              NS_ASSERT_MSG (theApcgTaskType.id() == theTaskType->ID(), "APCG XML says the task ID is "
                  << theApcgTaskType.id() << " but, task XML says the ID is " << theTaskType->ID() << "!");
              cerr << " (name: " << theTaskType->name () << " type: " << theTaskType->type () << " ";

              for (research::noc::application_mapping::unified_framework::schema::core::coreType::task_const_iterator i (
                  theCoreType->task ().begin ()); i != theCoreType->task ().end (); i++)
                {
                  research::noc::application_mapping::unified_framework::schema::core::coreType::task_type theCoreTaskType = *i;
                  if (theTaskType->type () == theCoreTaskType.type ())
                    {
                      cerr << "execution time: " << theCoreTaskType.execTime () << " (s) " << "power:"
                          << theCoreTaskType.power () << " (W) )" << endl;
                      break;
                    }
                }
            }
          cerr << endl;

          cerr << "Communications:" << endl;

          for (ctgType::communication_const_iterator i (theCtgType->communication ().begin ()); i
              != theCtgType->communication ().end (); i++)
            {
              ctgType::communication_type theCommunicationType = *i;

              cerr << "\tsource task: " << theCommunicationType.source ().id () << endl;
              for (research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_const_iterator
                  i (theCommunicationType.source ().deadline ().begin ()); i
                  != theCommunicationType.source ().deadline ().end (); i++)
                {
                  research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_type
                      deadline = *i;
                  cerr << "\t\tdeadline: " << deadline.type () << " at " << deadline << " (ms)" << endl;
                }

              cerr << "\tdestination task: " << theCommunicationType.destination ().id () << endl;
              for (research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_const_iterator
                  i (theCommunicationType.destination ().deadline ().begin ()); i
                  != theCommunicationType.destination ().deadline ().end (); i++)
                {
                  research::noc::application_mapping::unified_framework::schema::ctg::communicatingTaskType::deadline_type
                      deadline = *i;
                  cerr << "\t\tdeadline: " << deadline.type () << " at " << deadline << " (ms)" << endl;
                }

              cerr << "\tcommunication volume: " << theCommunicationType.volume () << " (bits)" << endl;

              cerr << endl;
            }
        }

    }
  catch (const xml_schema::exception& e)
    {
      cerr << e << endl;
      return 1;
    }
}
