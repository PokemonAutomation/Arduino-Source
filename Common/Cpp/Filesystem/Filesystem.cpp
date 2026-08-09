/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "FilePath.h"
#include "Filesystem.h"

namespace PokemonAutomation{
namespace Filesystem{



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



}
}
