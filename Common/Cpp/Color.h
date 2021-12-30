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


class Color{
public:
    constexpr Color() : m_argb(0) {}
    constexpr explicit Color(uint32_t argb) : m_argb(argb) {}

    constexpr operator bool() const{
        return m_argb != 0;
    }
    constexpr explicit operator uint32_t() const{
        return m_argb;
    }

private:
    uint32_t m_argb;
};


constexpr Color COLOR_WHITE(0xffffffff);
constexpr Color COLOR_BLACK(0xff000000);
constexpr Color COLOR_GRAY(0xff808080);

constexpr Color COLOR_RED(0xffff0000);
constexpr Color COLOR_GREEN(0xff00ff00);
constexpr Color COLOR_BLUE(0xff0000ff);

constexpr Color COLOR_MAGENTA(0xffff00ff);
constexpr Color COLOR_YELLOW(0xffffff00);
constexpr Color COLOR_CYAN(0xff00ffff);

constexpr Color COLOR_ORANGE(0xffffa500);
constexpr Color COLOR_PURPLE(0xff800080);

constexpr Color COLOR_DARKGREEN(0xff008000);
constexpr Color COLOR_DARKCYAN(0xff008080);




}
#endif
