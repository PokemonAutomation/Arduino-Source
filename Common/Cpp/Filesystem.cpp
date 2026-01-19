/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Filesystem.h"
#include <iostream>

namespace PokemonAutomation::Filesystem{

std::ostream& operator<<(std::ostream& stream, const Path& x){
    return stream << x.m_path;
}


}
