#ifndef ORG_SIMPLE_BITSHELPER_H
#define ORG_SIMPLE_BITSHELPER_H
/*
 * org-simple/BitsHelper.h
 *
 * Added by michel on 2024-01-02
 * Copyright (C) 2015-2024 Michel Fleur.
 * Source https://github.com/emmef/org-simple
 * Email org-simple@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bit>
#include <cstddef>
#include <type_traits>

template <typename number, unsigned separatorInterval>
concept isValidBitRenderConfiguration = std::is_integral_v<number> &&
                                        (separatorInterval == 0 || std::has_single_bit(separatorInterval));


/**
 * Gets the number of characters needed render the bits using \c renderBitsInfo.
 * @tparam number The type of number to display the bits for.
 * @tparam separator The separator character to be used if not '_'.
 * @tparam separatorInterval The separator interval in bits, that must be a
 * power of two.
 * @return The number
 */
template <typename number, unsigned separatorInterval = 0>
static constexpr unsigned renderBitsCharacters() requires
    isValidBitRenderConfiguration<number, separatorInterval> {
  const unsigned bits = sizeof(number) * 8;
  return bits + 1 + (separatorInterval ? bits / separatorInterval : 0);
}

/**
 * Renders the number in binary format into the specified buffer, that must at
 * least have a length of \c binaryDisplayLength, and returns a pointer to the
 * buffer after the rendered bits.
 *
 * @tparam number The type of number to display the bits for.
 * @tparam separator The separator character to be used if not '_'.
 * @tparam separatorInterval The separator interval in bits, that must be a
 * power of two.
 * @param num The number of the specified type.
 * @param buffer The buffer to write into.
 * @return A pointer to the buffer after the rendered bits.
 */
template <typename C, typename number, char separator = '_',
          unsigned separatorInterval = 0>
static const C *renderBitsInto(number num, C *buffer)  requires
    isValidBitRenderConfiguration<number, separatorInterval> {
  static constexpr int bits = sizeof(num) * 8;
  number test = number(sizeof(1) << (sizeof(num) * 8 - 1));
  if constexpr (separatorInterval) {
    int i;
    int j;
    for (i = 0, j = 0; i < bits; i++, test >>= 1) {
      if (i != 0 && (i % separatorInterval) == 0) {
        buffer[j++] = separator;
      }
      buffer[j++] = test & num ? '1' : '0';
    }
  } else {
    int i;
    for (i = 0; i < bits; i++, test >>= 1) {
      buffer[i] = test & num ? '1' : '0';
    }
  }
  return buffer + renderBitsCharacters<number, separatorInterval>();
}

/**
 * Renders the number in binary format a static buffer and returns a pointer to
 * that buffer.
 *
 * @tparam number The type of number to display the bits for.
 * @tparam separator The separator character to be used if not '_'.
 * @tparam separatorInterval The separator interval in bits, that must be a
 * power of two.
 * @param num The number of the specified type.
 * @return A pointer to the buffer with the rendered bits.
 */
template <typename number, char separator = '_', unsigned separatorInterval = 0>
static const char *renderBits(number num) requires
    isValidBitRenderConfiguration<number, separatorInterval>{
  static constexpr size_t length = renderBitsCharacters<number, separatorInterval>();
  static char buffer[length + 1];
  renderBitsInto<char, number, separator, separatorInterval>(num, buffer);
  buffer[length] = '\0';
  return buffer;
}

template <typename number, bool separators = true>
static const char *binary(number num) {
  return renderBits<number, '_', separators ? 4 : 0>(num);
}


#endif // ORG_SIMPLE_BITSHELPER_H
