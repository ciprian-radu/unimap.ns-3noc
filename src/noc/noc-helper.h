/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef NOCHELPER_H_
#define NOCHELPER_H_

#include "noc-channel.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/ascii-writer.h"

namespace ns3
{

  class NocChannel;

  class NocHelper : public Object
  {
  public:

    /**
     * \param os output stream
     * \param nodeid the id of the node to generate ascii output for.
     * \param deviceid the id of the device to generate ascii output for.
     *
     * Enable ascii output on the specified deviceid within the
     * specified nodeid if it is of type ns3::NocNetDevice and dump
     * that to the specified stdc++ output stream.
     */
    static void
    EnableAscii(std::ostream &os, uint32_t nodeid, uint32_t deviceid);

    /**
     * \param os output stream
     * \param d device container
     *
     * Enable ascii output on each device which is of the
     * ns3::NocNetDevice type and which is located in the input
     * device container and dump that to the specified
     * stdc++ output stream.
     */
    static void
    EnableAscii(std::ostream &os, NetDeviceContainer d);

    /**
     * \param os output stream
     * \param n node container
     *
     * Enable ascii output on each device which is of the
     * ns3::NocNetDevice type and which is located in one
     * of the input node and dump that to the specified
     * stdc++ output stream.
     */
    static void
    EnableAscii(std::ostream &os, NodeContainer n);

    /**
     * \param os output stream
     *
     * Enable ascii output on each device which is of the
     * ns3::NocNetDevice type and dump that to the specified
     * stdc++ output stream.
     */
    static void
    EnableAsciiAll(std::ostream &os);

    NetDeviceContainer
    Install(NodeContainer nodes, Ptr<NocChannel> channel);

    NetDeviceContainer
    Install(NodeContainer nodes);

    NetDeviceContainer
    Install2DMesh(NodeContainer nodes, uint32_t hSize);

    Ptr<NocNetDevice>
    FindNetDeviceByAddress (Mac48Address address);

  private:

    static void
    AsciiTxEvent(Ptr<AsciiWriter> writer, std::string path,
        Ptr<const Packet> packet);

    static void
    AsciiRxEvent(Ptr<AsciiWriter> writer, std::string path,
        Ptr<const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiEnqueueEvent(Ptr<AsciiWriter> writer, std::string path, Ptr<
        const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiDequeueEvent(Ptr<AsciiWriter> writer, std::string path, Ptr<
        const Packet> packet);

    /*
     * \internal
     */
    static void
    AsciiDropEvent(Ptr<AsciiWriter> writer, std::string path,
        Ptr<const Packet> packet);

    NetDeviceContainer m_devices;
  };

} // namespace ns3

#endif /* NOCHELPER_H_ */
