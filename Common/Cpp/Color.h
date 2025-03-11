/*  Color
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A very lightweight color class to avoid pulling in <QColor>.
 *
 */

#ifndef PokemonAutomation_Color_H
#define PokemonAutomation_Color_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{


constexpr inline uint32_t combine_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b){
    return (((uint32_t)a << 8 | r) << 8 | g) << 8 | b;
}

constexpr inline uint32_t combine_rgb(uint8_t r, uint8_t g, uint8_t b){
    return (((uint32_t)255 << 8 | r) << 8 | g) << 8 | b;
}


class Color{
public:
    constexpr Color() : m_argb(0) {}
    constexpr explicit Color(uint32_t argb) : m_argb(argb) {}
    constexpr Color(uint8_t red, uint8_t green, uint8_t blue) : m_argb(combine_rgb(red, green, blue)) {}
    constexpr Color(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue) : m_argb(combine_argb(alpha, red, green, blue)) {}

    constexpr explicit operator bool() const{
        return m_argb != 0;
    }
    constexpr explicit operator uint32_t() const{
        return m_argb;
    }
    bool operator<(Color color) const{
        return m_argb < color.m_argb;
    }
    bool operator==(Color color) const{
        return m_argb == color.m_argb;
    }
    bool operator!=(Color color) const{
        return m_argb != color.m_argb;
    }

    uint8_t alpha   () const { return (uint8_t)(m_argb >> 24); }
    uint8_t red     () const { return (uint8_t)(m_argb >> 16); }
    uint8_t green   () const { return (uint8_t)(m_argb >>  8); }
    uint8_t blue    () const { return (uint8_t)(m_argb >>  0); }

    // Example: "[0xFFFDBD00 A=255 R=253 G=189 B=00]"
    std::string to_string() const;

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

constexpr Color COLOR_GREEN2(0xff00aa00);




}
#endif
