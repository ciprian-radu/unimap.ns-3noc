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

#include <iostream>

#include "ns3/core-module.h"
#include "ns3/helper-module.h"
#include "ns3/node-module.h"
#include "ns3/simulator-module.h"
#include "ns3/noc-header.h"
#include "ns3/noc-packet.h"

using namespace ns3;
using namespace std;

static uint8_t payloadSize = 9;

static void
GenerateTraffic(Ptr<Socket> socket, uint32_t size)
{
  if (size > 0)
    {
      cout << "sent at=" << Simulator::Now().GetSeconds() << "s, sending "
          << payloadSize << " bytes, bytes remained to send=" << size << endl;

      //  Ptr<Packet> packet = Create<Packet> (size);
      //  NocHeader nocHeader;
      //
      //  nocHeader.SetXDistance(2);
      //  nocHeader.SetYDistance(3);
      //  nocHeader.SetSourceX(3);
      //  nocHeader.SetSourceY(4);
      //  nocHeader.SetSubdataId(32);
      //  nocHeader.SetPeGroupAddress(15);
      //  nocHeader.SetDataFlitCount(9);
      //
      //  // copy the header into the packet
      //  packet->AddHeader (nocHeader);

      // using the NocPacket is better because a NocHeader is automatically used
      // and the payload size of the packet is automatically inferred from the last field of the header
      Ptr<NocPacket> packet = Create<NocPacket> (2, 3, 3, 4, payloadSize);

      socket->Send(packet);
      Simulator::Schedule(Seconds(0.5), &GenerateTraffic, socket, size
          - payloadSize);
    }
  else
    {
      socket->Close();
    }
}

static void
SocketPrinter(Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while (packet = socket->Recv())
    {
      cout << "received at=" << Simulator::Now().GetSeconds() << "s, bytes="
          << packet->GetSize() << endl;
      cout << "\tprinting packet: " << (*packet) << endl;
    }
}

static void
PrintTraffic(Ptr<Socket> socket)
{
  socket->SetRecvCallback(MakeCallback(&SocketPrinter));
}

void
RunSimulation(void)
{
  // without the following line, the Print method from the Packet class
  // will not do anything (because registering packet metadata is disabled by default)
  PacketMetadata::Enable();

  NodeContainer c;
  c.Create(1);

  InternetStackHelper internet;
  internet.Install(c);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket(c.Get(0), tid);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 80);
  sink->Bind(local);

  Ptr<Socket> source = Socket::CreateSocket(c.Get(0), tid);
  InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetLoopback(), 80);
  source->Connect(remote);

  GenerateTraffic(source, payloadSize * 5);
  PrintTraffic(sink);

  Simulator::Run();

  Simulator::Destroy();
}

int
main(int argc, char *argv[])
{
  RunSimulation();

  return 0;
}
