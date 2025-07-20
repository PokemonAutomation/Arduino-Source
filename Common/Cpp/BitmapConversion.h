/*  Bitmap Conversion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Functions to:
 *  - convert vector<bool> to string and back
 *  - convert vector<bool> to base64 string and back.
 * 
 *  The Base64 algorithm is originally from René Nyffenegger's public domain base64 code
 *  (https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/)
 *
 *  These changes were made:
 *  1. String input/output interface
 *  2. Proper padding handling
 *  3. Explicit character validation
 */

#ifndef PokemonAutomation_BitmapCompression_H
#define PokemonAutomation_BitmapCompression_H

#include <string>
#include <vector>

std::string pack_bits_to_string(const std::vector<bool> &vec);

std::vector<bool> unpack_bits_from_string(const std::string &packed, size_t original_size);

std::string pack_bit_vector_to_base64(const std::vector<bool> &vec);

std::vector<bool> unpack_bit_vector_from_base64(const std::string &b64, size_t original_size);

#endif
