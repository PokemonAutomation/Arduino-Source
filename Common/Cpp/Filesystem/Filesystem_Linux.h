/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <unistd.h>
#include <limits.h>
#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_binary_path(){
    char buffer[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (count != -1) {
        return Path(std::string(buffer, count)).parent_path();
    }
    return Path();
}
Path application_working_path(){
    return application_binary_path();
}



}
}
