/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mach-o/dyld.h>
#include <limits.h>
#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_directory(){
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return std::filesystem::canonical(std::filesystem::path(buffer)).parent_path();
    }
    return Path();
}



}
}
