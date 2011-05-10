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

#ifndef NOCREGISTRY_H_
#define NOCREGISTRY_H_

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/noc-topology.h"

namespace ns3
{

  class NocTopology;

  /**
   * \brief Registry for globally accessible values
   * \detail This class follows the Singleton design pattern.
   *         Registry values are statically defined in this class,
   *         new registry values cannot be defined at runtime
   *         (only existing registry values can be set).
   */
  class NocRegistry : public Object
  {
  public:

    static TypeId
    GetTypeId ();

    static Ptr<NocRegistry>
    GetInstance ();

  private:

    /**
     * Class TypeId must have access to the private constructor
     */
    friend class TypeId;

    NocRegistry ();                                   // Private constructor

    NocRegistry (const NocRegistry&);                 // Prevent copy-construction

    NocRegistry& operator= (const NocRegistry&);      // Prevent assignment

    /**
     * the flit size, in bits
     */
    uint32_t m_flitSize;

    /**
     *  the speedup used for routing a data packet, as compared to a head packet
     */
    int m_dataFlitSpeedup;

    /**
     * the clock period with which events must be generated.
     * It is used only with a synchronous NoC.
     */
    Time m_globalClock;

    /**
     * the NoC topology
     */
    Ptr<NocTopology> m_nocTopology;

    /**
     * the NoC topology number of dimensions
     */
    uint32_t m_nocTopologyDimension;

  };

} // namespace ns3

#endif /* NOCREGISTRY_H_ */
