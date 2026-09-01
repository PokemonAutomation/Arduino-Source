/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <Windows.h>
#include "FilePath.h"
#include "Filesystem.h"

namespace PokemonAutomation{
namespace Filesystem{


Path application_binary_path(){
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer);
}
Path application_install_path(){
    return application_binary_directory();
}
Path application_scratch_path(){
    return current_path().string_slash_normalized() + "/";
}



}
}
