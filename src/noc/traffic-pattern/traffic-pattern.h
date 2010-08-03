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

#ifndef TRAFFIC_PATTERN_H
#define TRAFFIC_PATTERN_H

#include "ns3/object.h"
#include "ns3/type-id.h"
#include "ns3/ptr.h"
#include "ns3/random-variable.h"

namespace ns3 {

/**
 *
 * \brief Traffic patterns for applications
 */
class TrafficPattern : public Object
{
public:

  TrafficPattern ();

  virtual ~TrafficPattern();

  static TypeId GetTypeId ();

  /**
   * \brief Represents a given positive integer number in base 2 and transposes its bits
   * (the second half part of the bits are put in front of the first half).
   * The number must be represented on maximum 32 bits.
   *
   * \param the number
   *
   * \return the number which is obtained by transposing the bits of the given number
   */
  static uint32_t
  MatrixTransposeBits (uint32_t number);

  /**
   * \brief Represents a given positive integer number in base 2 and transposes its bits
   * (the second half part of the bits are put in front of the first half).
   * The number must be represented on maximum 32 bits.
   * Additionally, you can specify how many bits to use for the binary representation.
   *
   * \param number the number
   *
   * \param size the size (will be checked to be >= 1 and <= 32)
   *
   * \return the number which is obtained by transposing the bits of the given number
   */
  static uint32_t
  MatrixTransposeBits (uint32_t number, uint8_t size);

  /**
   * \brief Represents a given positive integer number in base 2 and complements its bits
   * (0 becomes 1 and 1 becomes 0). The number must be represented on maximum 32 bits.
   *
   * \param the number
   *
   * \return the number which is obtained by complementing the bits of the given number
   */
  static uint32_t
  ComplementBits (uint32_t number);

  /**
   * \brief Represents a given positive integer number in base 2 and complements its bits
   * (0 becomes 1 and 1 becomes 0). The number must be represented on maximum 32 bits.
   * Additionally, you can specify how many bits to use for the binary representation.
   *
   * \param number the number
   *
   * \param size the size (will be checked to be >= 1 and <= 32)
   *
   * \return the number which is obtained by complementing the bits of the given number
   */
  static uint32_t
  ComplementBits (uint32_t number, uint8_t size);

  /**
   * \brief Represents a given positive integer number in base 2 and reverses its bits
   * (the last bit becomes the first and so on). The number must be represented on maximum 32 bits.
   *
   * \param number the number
   *
   * \return the number which is obtained by reversing the bits of the given number
   */
  static uint32_t
  ReverseBits (uint32_t number);

  /**
   * \brief Represents a given positive integer number in base 2 and reverses its bits
   * (the last bit becomes the first and so on). The number must be represented on maximum 32 bits.
   *
   * \param number the number
   *
   * \param size the size (will be checked to be >= 1 and <= 32)
   *
   * \return the number which is obtained by reversing the bits of the given number
   */
  static uint32_t
  ReverseBits (uint32_t number, uint8_t size);

  /**
   * Returns a random unsigned integer from the interval [min, max] including both ends.
   *
   * \param min low end of the range
   * \param max end of the range
   *
   * \return a random unsigned integer value.
   *
   */
  uint32_t
  GetUniformRandomNumber (uint32_t min, uint32_t max);

private:

  UniformVariable m_uniformVariable;

};

} // namespace ns3

#endif

