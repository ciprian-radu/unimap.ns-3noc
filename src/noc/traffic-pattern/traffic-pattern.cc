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

#include "traffic-pattern.h"
#include "ns3/log.h"
#include <cstdlib>
#include <bitset>
#include "math.h"

NS_LOG_COMPONENT_DEFINE ("TrafficPattern");

using namespace std;

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (TrafficPattern);

  TrafficPattern::TrafficPattern()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  TrafficPattern::~TrafficPattern()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  TypeId
  TrafficPattern::GetTypeId(void)
  {
    static TypeId
        tid = TypeId("ns3::TrafficPattern")
            .SetParent<Object> ()
            .AddConstructor<TrafficPattern> ()
            ;
    return tid;
  }

  uint32_t
  TrafficPattern::MatrixTransposeBits (uint32_t number)
  {
    NS_LOG_FUNCTION (number);

    std::bitset<32> b(number);
    double log = 0;
    if (number > 0)
      {
        log = log2(number);
      }

    return MatrixTransposeBits(number, (uint8_t)floor (log + 1));
  }

  uint32_t
  TrafficPattern::MatrixTransposeBits (uint32_t number, uint8_t size)
  {
    NS_LOG_FUNCTION (number << (int) size);
    NS_ASSERT_MSG(size >= 1 && size <= 32, "The size must be <= 1 and <= 32");

    std::bitset<32> b(number);
    std::string binary(b.to_string().substr(32 - size));
    std::bitset<32> bits(binary);
    for (unsigned int i = 0; i < binary.size() / 2; ++i)
      {
      bool leftBit = bits.test (i);
      bool rightBit = bits.test (binary.size () / 2 + i);
      bits.set (i, rightBit);
      bits.set (binary.size () / 2 + i, leftBit);
      }
    uint32_t transposedNumber = bits.to_ulong();
    NS_LOG_DEBUG(number << " " << b.to_string().substr(32 - size) << " "
        << transposedNumber << " " << bits.to_string());

    return transposedNumber;
  }

  uint32_t
  TrafficPattern::ComplementBits (uint32_t number)
  {
    NS_LOG_FUNCTION (number);

    std::bitset<32> b(number);
    double log = 0;
    if (number > 0)
      {
        log = log2(number);
      }

    return ComplementBits(number, (uint8_t)floor (log + 1));
  }

  uint32_t
  TrafficPattern::ComplementBits (uint32_t number, uint8_t size)
  {
    NS_LOG_FUNCTION (number << (int) size);
    NS_ASSERT_MSG(size >= 1 && size <= 32, "The size must be <= 1 and <= 32");

    std::bitset<32> b(number);
    std::string binary(b.to_string().substr(32 - size));
    std::bitset<32> bits(binary);
    for (unsigned int i = 0; i < binary.size(); ++i)
      {
        bits.flip(i);
      }
    uint32_t reversedNumber = bits.to_ulong();
    NS_LOG_DEBUG(number << " " << b.to_string().substr(32 - size) << " "
        << reversedNumber << " " << bits.to_string());

    return reversedNumber;
  }

  uint32_t
  TrafficPattern::ReverseBits (uint32_t number)
  {
    NS_LOG_FUNCTION (number);

    std::bitset<32> b(number);
    double log = 0;
    if (number > 0)
      {
        log = log2(number);
      }

    return ReverseBits (number, (uint8_t)floor (log + 1));
  }

  uint32_t
  TrafficPattern::ReverseBits (uint32_t number, uint8_t size)
  {
    NS_LOG_FUNCTION (number << (int) size);
    NS_ASSERT_MSG(size >= 1 && size <= 32, "The size must be <= 1 and <= 32");

    std::bitset<32> b(number);
    std::string binary(b.to_string().substr(32 - size));
    std::bitset<32> bits(binary);
    for (unsigned int i = 0; i < binary.size() / 2; ++i)
      {
        bool leftBit = bits.test (i);
        bool rightBit = bits.test (binary.size () - i - 1);
        bits.set (i, rightBit);
        bits.set (binary.size () - i - 1, leftBit);
      }
    uint32_t reversedNumber = bits.to_ulong();
    NS_LOG_DEBUG(number << " " << b.to_string().substr(32 - size) << " "
        << reversedNumber << " " << bits.to_string());

    return reversedNumber;
  }

  uint32_t
  TrafficPattern::GetUniformRandomNumber (uint32_t min, uint32_t max)
  {
    return m_uniformVariable.GetInteger (min, max);
  }

} // Namespace ns3
