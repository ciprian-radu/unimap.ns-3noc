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

#ifndef __NocValue_H__
#define __NocValue_H__


#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/ptr.h"


namespace ns3 {


/**
 * \brief Helper class which allows a 32-bit unsigned integer value to be represented as an ns-3 Object
 */
class NocValue : public Object
{
public:
  static TypeId GetTypeId (void);

  NocValue ();

  NocValue (uint32_t value);

  virtual ~NocValue ();

  uint32_t
  GetValue () ;

  void
  SetValue(uint32_t value);

private:

  uint32_t m_value;

};

} //namespace ns3
#endif
