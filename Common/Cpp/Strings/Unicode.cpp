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

//
//  UTF-8
//
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

template <typename CharType>
void append_to_utf8(
    std::basic_string<CharType>& str,
    uint32_t codepoint,
    const CharType* replacement
){
    if (codepoint <= 0x7f){
        str += (char)codepoint;
        return;
    }
    if (codepoint > MAX_CODEPOINT){
        str += replacement;
        return;
    }

    int bytes = 0;
    bytes = codepoint <= 0x001fffff ? 4 : bytes;
    bytes = codepoint <= 0x0000ffff ? 3 : bytes;
    bytes = codepoint <= 0x000007ff ? 2 : bytes;
    str.reserve(str.size() + bytes);

    if (bytes == 0){
        str += replacement;
        return;
    }

    int shift = (bytes - 1) * 6;
    unsigned char ch = ((1 << bytes) - 1) << (8 - bytes);
    ch |= codepoint >> shift;
    str += ch;
    codepoint <<= 32 - shift;

    for (int c = 1; c < bytes; c++){
        str += 0x80 | (codepoint >> 26);
        codepoint <<= 6;
    }
}



//
//  UTF-16
//
uint32_t utf16_to_unicode(const char16_t*& str){
    uint32_t H = str[0];
    if (H < 0xd800 || H > 0xdfff){
        str++;
        return H;
    }

    if (H < 0xd800 || 0xdc00 <= H){
        str++;
        return REPLACEMENT;
    }

    uint32_t L = str[1];
    if (L < 0xdc00 || 0xe000 <= L){
        str++;
        return REPLACEMENT;
    }

    H = (uint32_t)(H - 0xd800) << 10;
    L = str[1] - 0xdc00;

    str += 2;
    return (L | H) + 0x10000;
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



//
//  Conversions
//

std::u8string utf8_to_utf8(const std::string& str){
    return std::u8string(str.begin(), str.end());
}
std::string utf8_to_str(const std::u8string& str){
    return std::string(str.begin(), str.end());
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
std::u16string utr8_to_utf16(const std::u8string& str){
    std::u16string out;
    const char* utf8 = (const char*)str.c_str();
    const char* stop = utf8 + str.size();
    while (utf8 < stop){
        append_to_utf16(out, utf8_to_unicode(utf8));
    }
    return out;
}
std::string utr16_to_str(const std::u16string& str){
    std::string out;
    const char16_t* utf16 = str.c_str();
    const char16_t* stop = utf16 + str.size();
    while (utf16 < stop){
        append_to_utf8(out, utf16_to_unicode(utf16), "\xef\xbf\xbd");
    }
    return out;
}
std::u8string utr16_to_utf8(const std::u16string& str){
    std::u8string out;
    const char16_t* utf16 = str.c_str();
    const char16_t* stop = utf16 + str.size();
    while (utf16 < stop){
        append_to_utf8(out, utf16_to_unicode(utf16), u8"\ufffd");
    }
    return out;
}

std::u32string utf8_to_utf32(const std::string& str){
    std::u32string out;
    const char* utf8 = str.c_str();
    const char* stop = utf8 + str.size();
    while (utf8 < stop){
        out += utf8_to_unicode(utf8);
    }
    return out;
}
std::u32string utf8_to_utf32(const std::u8string& str){
    std::u32string out;
    const char* utf8 = (const char*)str.c_str();
    const char* stop = utf8 + str.size();
    while (utf8 < stop){
        out += utf8_to_unicode(utf8);
    }
    return out;
}
std::string utf32_to_str(const std::u32string& str){
    std::string out;
    for (char32_t ch : str){
        append_to_utf8(out, ch, "\xef\xbf\xbd");
    }
    return out;
}
std::u8string utf32_to_utf8(const std::u32string& str){
    std::u8string out;
    for (char32_t ch : str){
        append_to_utf8(out, ch, u8"\ufffd");
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

