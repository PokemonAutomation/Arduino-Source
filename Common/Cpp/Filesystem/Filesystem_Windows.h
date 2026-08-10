/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <Windows.h>
#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_binary_path(){
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}
Path application_working_path(){
    return application_binary_path();
}



}
}
