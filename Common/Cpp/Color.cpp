/*  Color
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A very lightweight color class to avoid pulling in <QColor>.
 *
 */

#include <sstream>
#include <iomanip>
#include "Color.h"

namespace PokemonAutomation{

std::string Color::to_string() const{
    std::ostringstream os;
    os << "[0x" << std::internal << std::setfill('0');
    os << std::hex << std::uppercase << std::setw(8) << m_argb << std::dec;
    os << " A=" << std::setw(2) << int(alpha());
    os << " R=" << std::setw(2) << int(red());
    os << " G=" << std::setw(2) << int(green());
    os << " B=" << std::setw(2) << int(blue()) << "]";
    return os.str();
}

}

