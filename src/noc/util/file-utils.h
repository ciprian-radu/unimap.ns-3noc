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

#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_

#include "ns3/object.h"
#include "ns3/ptr.h"

namespace ns3
{

  class FileUtils : public Object
  {
  public:

    static TypeId
    GetTypeId ();

    /**
     * Recursively creates all the directories so that the given directory path can be created.
     * The code is based on http://nion.modprobe.de/tmp/mkdir.c
     *
     * \param path the directory path
     */
    static void
    MkdirRecursive (const char *path);

  private:

    /**
     * Class TypeId must have access to the private constructor
     */
    friend class TypeId;

    FileUtils (); // Private constructor

    FileUtils (const FileUtils&); // Prevent copy-construction

    FileUtils&
    operator= (const FileUtils&); // Prevent assignment

  };

} // namespace ns3

#endif /* FILE_UTILS_H_ */
