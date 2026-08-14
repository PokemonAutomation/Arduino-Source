/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CompileTimeBackends.h"
#include "FilePath.h"
#include "Filesystem.h"

#ifdef PA_FILE_SYSTEM_BACKEND_OS_SPECIFIC
#if 0
#elif _WIN32
#include "Filesystem_Windows.h"
#elif __linux__
#include "Filesystem_Linux.h"
#elif __APPLE__
#include "Filesystem_Mac.h"
#else
#error "No file backend specified."
#endif
#endif

#ifdef PA_FILE_SYSTEM_BACKEND_Qt
#include "Filesystem_Qt.h"
#endif

namespace PokemonAutomation{
namespace Filesystem{



Path absolute(const Path& path){
    return std::filesystem::absolute(path.stdpath());
}

bool exists(const Path& path){
    return std::filesystem::exists(path.stdpath());
}

bool create_directories(const Path& path){
    return std::filesystem::create_directories(path.stdpath());
}

std::uintmax_t remove_all(const Path& path){
    return std::filesystem::remove_all(path.stdpath());
}

bool copy_file(const Path& from, const Path& to){
    return std::filesystem::copy_file(from.stdpath(), to.stdpath());
}

std::uintmax_t file_size(const Path& path){
    return std::filesystem::file_size(path.stdpath());
}

std::uintmax_t file_size(const Path& path, std::error_code& ec){
    return std::filesystem::file_size(path.stdpath(), ec);
}

void rename(const Path& old_path, const Path& new_path){
    std::filesystem::rename(old_path.stdpath(), new_path.stdpath());
}

void rename(const Path& old_path, const Path& new_path, std::error_code& ec){
    std::filesystem::rename(old_path.stdpath(), new_path.stdpath(), ec);
}

Path current_path(){
    return std::filesystem::current_path();
}

Path application_binary_name(){
    return application_binary_path().filename();
}
Path application_binary_directory(){
    return application_binary_path().parent_path();
}




}
}
