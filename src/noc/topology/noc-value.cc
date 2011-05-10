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

// Implementation for ns3 NocValue base class.
#include "noc-value.h"

using namespace std;

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NocValue);

// NocValue Methods

  TypeId
  NocValue::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::NocValue")
        .SetParent<Object> ()
        .AddAttribute ("Value", "What value does it have",
              UintegerValue (0), MakeUintegerAccessor (&NocValue::m_value),
              MakeUintegerChecker<uint32_t> (1, 127))
    ;
    return tid;
  }

  // \brief NocValue Constructor
  NocValue::NocValue ()
  {
  }

  NocValue::NocValue (uint32_t value)
  {
    m_value = value;
  }

  // \brief NocValue Destructor
  NocValue::~NocValue ()
  {
  }

  uint32_t
  NocValue::GetValue ()
  {
    return m_value;
  }

  void
  NocValue::SetValue (uint32_t value)
  {
    m_value = value;
  }

} //namespace ns3


