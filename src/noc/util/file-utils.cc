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

#include "ns3/file-utils.h"
#include "ns3/log.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

NS_LOG_COMPONENT_DEFINE ("FileUtils");

namespace ns3
{
  NS_OBJECT_ENSURE_REGISTERED ( FileUtils);

  TypeId
  FileUtils::GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::FileUtils") .SetParent<Object> ();
    return tid;
  }

  void
  FileUtils::MkdirRecursive (const char *path)
  {
    NS_LOG_FUNCTION (path);

    char opath[256];
    char *p;
    size_t len;

    strncpy (opath, path, sizeof(opath));
    len = strlen (opath);
    if (opath[len - 1] == '/')
      {
        opath[len - 1] = '\0';
      }
    for (p = opath; *p; p++)
      {
        if (*p == '/')
          {
            *p = '\0';
            // Test for existence
            if (access (opath, F_OK))
              {
                // S_IRWXU = Read, write, and execute by owner.
                int code = mkdir (opath, S_IRWXU);
                if (code == 0)
                  {
                    NS_LOG_DEBUG ("Created directory " << opath);
                  }
                else
                  {
                    NS_LOG_ERROR ("Cannot create directory " << opath << " (error code " << code << ")");
                  }
              }
            *p = '/';
          }
      }
    // if path is not terminated with /
    if (access (opath, F_OK))
      {
        int code = mkdir (opath, S_IRWXU);
        if (code == 0)
          {
            NS_LOG_DEBUG ("Created directory " << opath);
          }
        else
          {
            NS_LOG_ERROR ("Cannot create directory " << opath << " (error code " << code << ")");
          }
      }
  }
} // namespace ns3
