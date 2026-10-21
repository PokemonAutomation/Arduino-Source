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
Path application_user_data_path(){
    //  %APPDATA% (C:\Users\<user>\AppData\Roaming) is the per-user application
    //  data folder on Windows. Use the wide-character APIs to correctly handle
    //  non-ASCII user names.
    Path base;
    {
        wchar_t buffer[MAX_PATH];
        const DWORD size = GetEnvironmentVariableW(L"APPDATA", buffer, MAX_PATH);
        if (size > 0 && size < MAX_PATH){
            base = Path(std::filesystem::path(buffer));
        }
    }
    if (base.empty()){
        wchar_t buffer[MAX_PATH];
        const DWORD size = GetEnvironmentVariableW(L"USERPROFILE", buffer, MAX_PATH);
        if (size > 0 && size < MAX_PATH){
            base = Path(std::filesystem::path(buffer)) / "AppData" / "Roaming";
        }
    }
    if (base.empty()){
        base = current_path();
    }
    return base / application_binary_name().stem().string();
}



}
}
