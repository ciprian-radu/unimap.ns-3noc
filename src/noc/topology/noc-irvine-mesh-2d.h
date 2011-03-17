/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 - 2011
 *               - Advanced Computer Architecture and Processing Systems (ACAPS),
 *               						Lucian Blaga University of Sibiu, Romania
 *               - Systems and Networking, University of Augsburg, Germany
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

#ifndef NOCIRVINEMESH2D_H_
#define NOCIRVINEMESH2D_H_

#include "ns3/noc-topology.h"
#include "ns3/noc-channel.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/trace-helper.h"
#include "ns3/object-factory.h"

namespace ns3
{

  class NocIrvineMesh2D : public NocTopology
  {
  public:

    static TypeId
    GetTypeId();

    NocIrvineMesh2D ();

    virtual
    ~NocIrvineMesh2D ();

    NetDeviceContainer
    Install (NodeContainer nodes);

    vector<uint8_t>
    GetDestinationRelativeDimensionalPosition (uint32_t sourceNodeId, uint32_t destinationNodeId);

    void
    SaveTopology (NodeContainer nodes, string directoryPath);

  private:

    /**
     * how many nodes the Irvine 2D mesh will have on one horizontal line
     */
    int m_hSize;

  };

} // namespace ns3

#endif /* NOCIRVINEMESH2D_H_ */
