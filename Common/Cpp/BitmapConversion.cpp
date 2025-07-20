// Base64 Originally from René Nyffenegger's public domain base64 code
// (https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/)
//
// These changes were made:
// 1. String input/output interface
// 2. Proper padding handling
// 3. Explicit character validation

#include "BitmapConversion.h"

static const std::string BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

bool base64_encode(std::string &out, const std::string &in) {
  int i = 0, j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];
  out.clear();

  for (auto c : in) {
    char_array_3[i++] = c;
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; i++)
        out += BASE64_CHARS[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; j < i + 1; j++)
      out += BASE64_CHARS[char_array_4[j]];

    while (i++ < 3)
      out += '=';
  }

  return true;
}

bool base64_decode(std::string &out, const std::string &in) {
  int i = 0, j = 0;
  unsigned char char_array_4[4], char_array_3[3];
  out.clear();

  for (auto c : in) {
    if (c == '=')
      break;

    if (!is_base64(c))
      return false;

    char_array_4[i++] = c;
    if (i == 4) {
      for (i = 0; i < 4; i++)
        char_array_4[i] = (unsigned char)BASE64_CHARS.find(char_array_4[i]);

      char_array_3[0] =
          (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] =
          ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

      for (i = 0; i < 3; i++)
        out += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++)
      char_array_4[j] = 0;

    for (j = 0; j < 4; j++)
      char_array_4[j] = (unsigned char)BASE64_CHARS.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] =
        ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);

    for (j = 0; j < i - 1; j++)
      out += char_array_3[j];
  }

  return true;
}

std::string pack_bits_to_string(const std::vector<bool> &vec) {
  std::string packed;
  if (vec.empty())
    return packed;

  packed.reserve((vec.size() + 7) / 8);
  for (size_t i = 0; i < vec.size(); i += 8) {
    unsigned char byte = 0;
    for (size_t j = 0; j < 8 && (i + j) < vec.size(); ++j) {
      if (vec[i + j]) {
        byte |= (1 << (7 - j));
      }
    }
    packed.push_back(byte);
  }
  return packed;
}

std::vector<bool> unpack_bits_from_string(const std::string &packed,
                                          size_t original_size) {
  std::vector<bool> vec;
  vec.reserve(original_size);
  for (size_t i = 0; i < packed.size(); ++i) {
    unsigned char byte = packed[i];
    for (int j = 7; j >= 0 && vec.size() < original_size; --j) {
      vec.push_back((byte >> j) & 1);
    }
  }
  return vec;
}

std::string pack_bit_vector_to_base64(const std::vector<bool> &vec) {
  std::string packed = pack_bits_to_string(vec);
  std::string b64;
  base64_encode(b64, packed);
  return b64;
}

std::vector<bool> unpack_bit_vector_from_base64(const std::string &b64,
                                                size_t original_size) {
  std::string packed;
  base64_decode(packed, b64);
  return unpack_bits_from_string(packed, original_size);
}
