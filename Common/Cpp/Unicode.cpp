/*  Unicode
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "Unicode.h"
#include <cstdint>

namespace PokemonAutomation{


const uint32_t MAX_CODEPOINT = 0x10ffff;
const uint32_t REPLACEMENT = 0xfffd;

void utf8_skip_to_next_codepoint(const char*& str){
    while (true){
        str++;
        unsigned ch = str[0];
        if ((ch & 0x80u) == 0){
            return;
        }
        if ((ch & 0xc0u) == 0xc0u){
            return;
        }
    }
}
uint32_t utf8_to_unicode(const char*& str){
    unsigned char ch = str[0];
    if (ch <= 0x7f){
        str++;
        return ch;
    }

    uint32_t codepoint;

    if (ch < 0xc0){
        utf8_skip_to_next_codepoint(str);
        return REPLACEMENT;
    }

    int bytes = 0;
    bytes = ch <= 0xf7 ? 4 : bytes;
    bytes = ch <= 0xef ? 3 : bytes;
    bytes = ch <= 0xdf ? 2 : bytes;

    if (bytes == 0){
        utf8_skip_to_next_codepoint(str);
        return REPLACEMENT;
    }

    ch &= ((unsigned char)1 << (7 - bytes)) - 1;
    codepoint = ch;

    for (int c = 1; c < bytes; c++){
        ch = str[c];
        if ((ch & 0xc0) != 0x80){
            utf8_skip_to_next_codepoint(str);
            return REPLACEMENT;
        }
        codepoint <<= 6;
        codepoint |= ch & 0x3f;
    }

    if (codepoint > MAX_CODEPOINT){
        utf8_skip_to_next_codepoint(str);
        return REPLACEMENT;
    }

    str += bytes;
    return codepoint;
}
void append_to_utf16(std::u16string& str, uint32_t codepoint){
    if (codepoint < 0xffff){
        str += (char16_t)codepoint;
        return;
    }
    str.reserve(str.size() + 2);

    codepoint -= 0x10000;
    str += (char16_t)(codepoint >> 10) + 0xd800;
    str += (char16_t)(codepoint & 0x3ff) + 0xdc00;
}

std::u16string utf8_to_utf16(const std::string& str){
    std::u16string out;
    const char* utf8 = str.c_str();
    const char* stop = utf8 + str.size();
    while (utf8 < stop){
        append_to_utf16(out, utf8_to_unicode(utf8));
    }
    return out;
}

#ifdef _WIN32
std::wstring utf8_to_wstr(const std::string& str){
    std::u16string tmp(utf8_to_utf16(str));
    return std::wstring(tmp.begin(), tmp.end());
}
#endif



}

