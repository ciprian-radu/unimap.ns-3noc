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

#ifndef IRVINELOADROUTER_H_
#define IRVINELOADROUTER_H_

#include "ns3/irvine-router.h"
#include "ns3/load-router-component.h"

namespace ns3
{

  /**
   * \brief Irvine router with load information
   */
  class IrvineLoadRouter : public IrvineRouter
  {
  public:

    static TypeId
    GetTypeId ();

    /**
     * Default constructor
     *
     * Note that you must specify a LoadRouterComponent manually when using this constructor
     *
     *
     * \see #SetLoadComponent(Ptr<LoadRouterComponent> loadComponent)
     */
    IrvineLoadRouter ();

    /**
     * Constructor
     *
     * \param loadComponent the load router component
     */
    IrvineLoadRouter (Ptr<LoadRouterComponent> loadComponent);

    virtual
    ~IrvineLoadRouter ();

    virtual void
    CreateLoadComponent (TypeId loadComponentTypeId);

    /**
     * Adds the load received from a neighbor router (marked by its topological direction)
     *
     * \param load the load information received from a neighbor
     * \param sourceDevice the net device from which the load information came
     */
    void
    AddNeighborLoad (int load, Ptr<NocNetDevice> sourceDevice);

    /**
     * Retrieves the load of the router which is a neighbor to this router
     * (by being connected to this router through sourceDevice)
     *
     * \param sourceDevice the net device of this router
     *
     * \return the load
     */
    int
    GetNeighborLoad (Ptr<NocNetDevice> sourceDevice);

    /**
     * Retrieves the load of the router which is a neighbor to this router,
     * in the specified direction.
     *
     * \param sourceDevice the net device of this router (it marks where the packet is in this router)
     * \param direction the direction of the neighbor
     * \param dimension the topological dimension
     *
     * \return the load
     */
    int
    GetNeighborLoad (Ptr<NocNetDevice> sourceDevice, int direction, int dimension);

  private:

    /**
     * the load of the neighbor router connected to the left North channel
     */
    int m_northLeftLoad;

    /**
     * the load of the neighbor router connected to the right North channel
     */
    int m_northRightLoad;

    /**
     * the load of the neighbor router connected to the East channel
     */
    int m_eastLoad;

    /**
     * the load of the neighbor router connected to the left South channel
     */
    int m_southLeftLoad;

    /**
     * the load of the neighbor router connected to the right South channel
     */
    int m_southRightLoad;

    /**
     * the load of the neighbor router connected to the West channel
     */
    int m_westLoad;

  };

} // namespace ns3

#endif /* IRVINELOADROUTER_H_ */
