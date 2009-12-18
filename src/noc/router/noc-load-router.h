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

#ifndef NOCLOADROUTER_H_
#define NOCLOADROUTER_H_

#include "ns3/noc-router.h"

namespace ns3
{

  /**
   *
   * \brief Interface for the NoC router which uses network load information do perform routing
   *
   */
  class NocLoadRouter : public NocRouter
  {
  public:

    static TypeId
    GetTypeId ();

    NocLoadRouter (std::string name);

    virtual
    ~NocLoadRouter ();

    /**
     * \return the load of the router
     */
    virtual int
    GetLocalLoad () = 0;

    /**
     * Computes the load that is propagated from this router, in the specified direction.
     *
     * \param direction the direction
     *
     * \return the load for the specified direction
     */
    virtual int
    GetLoadForDirection (int direction) = 0;

  protected:

  private:


  };

} // namespace ns3

#endif /* NOCLOADROUTER_H_ */
