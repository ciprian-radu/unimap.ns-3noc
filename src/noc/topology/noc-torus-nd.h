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
 * Author: Andreea Gancea <andreea.gancea@ulbsibiu.ro>
 *
 */


#ifndef NocTorusND_H_
#define NocTorusND_H_

#include "ns3/noc-topology.h"
#include "ns3/noc-channel.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/trace-helper.h"
#include "ns3/object-factory.h"
#include "src/noc/util/noc-registry.h"
#include "noc-value.h"

namespace ns3
{

  class NocTorusND : public NocTopology
  {
  public:

    static TypeId
    GetTypeId();

    NocTorusND ();

    NocTorusND (vector<Ptr<NocValue> > Size);

    virtual
    ~NocTorusND ();

    NetDeviceContainer
    Install (NodeContainer nodes);

    vector<uint8_t>
    GetDestinationRelativeDimensionalPosition (uint32_t sourceNodeId, uint32_t destinationNodeId);

    uint32_t
    GetNumberOfNodes(int32_t dimension);

    void
    SaveTopology (NodeContainer nodes, string directoryPath);

  private:

    /**
     * how many nodes the nD Torus will have on each line
     */
    std::vector<Ptr<NocValue> > m_size;

    /**
     * how many dimensions the nD Torus will have
     */
    uint32_t m_value;

  };

} // namespace ns3

#endif /* NocTorusND_H_ */
