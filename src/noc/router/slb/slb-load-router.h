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

#ifndef SLBLOADROUTER_H_
#define SLBLOADROUTER_H_

#include "ns3/noc-load-router.h"

namespace ns3
{

  /**
   *
   * \brief Abstract load router for the Static Load-and-Bound routing algorithm.
   *
   */
  class SlbLoadRouter : public NocLoadRouter
  {
  public:

    static TypeId
    GetTypeId ();

    SlbLoadRouter ();

    virtual
    ~SlbLoadRouter ();

    /**
     * \return the load of the router
     */
    int
    GetLocalLoad ();

    /**
     * Computes the load that is propagated from this router, in the specified direction.
     *
     * \param direction the direction
     *
     * \return the load for the specified direction
     */
    int
    GetLoadForDirection (int direction);

  protected:

  private:


  };

} // namespace ns3

#endif /* SLBLOADROUTER_H_ */
