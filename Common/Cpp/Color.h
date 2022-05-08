/*  Color
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A very lightweight color class to avoid pulling in <QColor>.
 *
 */

#ifndef PokemonAutomation_Color_H
#define PokemonAutomation_Color_H

#include <stdint.h>

namespace PokemonAutomation{


constexpr inline uint32_t combine_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return (((uint32_t)a << 8 | r) << 8 | g) << 8 | b;
}

constexpr inline uint32_t combine_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (((uint32_t)255 << 8 | r) << 8 | g) << 8 | b;
}


class Color{
public:
    constexpr Color() : m_argb(0) {}
    constexpr explicit Color(uint32_t argb) : m_argb(argb) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b) : m_argb(combine_rgb(r, g, b)) {}
    constexpr Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b) : m_argb(combine_argb(a, r, g, b)) {}

    constexpr explicit operator bool() const{
        return m_argb != 0;
    }
    constexpr explicit operator uint32_t() const{
        return m_argb;
    }
    bool operator<(Color color) const{
        return m_argb < color.m_argb;
    }

    uint8_t a() const { return (uint8_t)(m_argb >> 24); }
    uint8_t r() const { return (m_argb & 0x00ff0000) >> 16; }
    uint8_t g() const { return (m_argb & 0x0000ff00) >> 8; }
    uint8_t b() const { return (m_argb & 0x000000ff); }

private:
    uint32_t m_argb;
};


constexpr Color COLOR_WHITE(0xffffffff);
constexpr Color COLOR_BLACK(0xff000000);
constexpr Color COLOR_GRAY(0xff808080);

constexpr Color COLOR_RED(0xffff0000);
constexpr Color COLOR_GREEN(0xff00ff00);
constexpr Color COLOR_BLUE(0xff0080ff);
constexpr Color COLOR_DARK_BLUE(0xff0000ff);    //  Hard to see on dark theme.

constexpr Color COLOR_MAGENTA(0xffff00ff);
constexpr Color COLOR_YELLOW(0xffffff00);
constexpr Color COLOR_CYAN(0xff00ffff);

constexpr Color COLOR_ORANGE(0xffffa500);
//constexpr Color COLOR_PURPLE(0xff800080);       //  Hard to see on dark theme.
constexpr Color COLOR_PURPLE(0xff8a2be2);

constexpr Color COLOR_DARKGREEN(0xff008000);
constexpr Color COLOR_DARKCYAN(0xff008080);




}
#endif
