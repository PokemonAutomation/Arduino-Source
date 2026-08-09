/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *  We disllow direct use of "std::filesystem" because it's too easy to mess up
 *  Unicode support.
 *
 *  In this project, we define "char*" and "std::string" as UTF-8. But this is
 *  not what happens in Windows as it uses ANSI or whatever the locale is.
 *
 *  This applies to all standard library functions as well as Window's own APIs.
 *
 *  Therefore, we cannot pass "char*"/"std::string" into any libraries. But this
 *  is too easy to do accidentally.
 *
 *  Therefore, we ban the direct use of "std::filesystem" in our code base and
 *  wrap them instead.
 *
 */

#ifndef PokemonAutomation_Filesystem_Filesystem_H
#define PokemonAutomation_Filesystem_Filesystem_H

#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{


//  Whether a path exists.
inline bool exists(const Path& path){
    return std::filesystem::exists(path.stdpath());
}

//  Create every missing directory along the path.
//  If all related directories already exist, return false. Otherwise return true.
inline bool create_directories(const Path& path){
    return std::filesystem::create_directories(path.stdpath());
}

//  Delete the contents of the path (if it is a directory) and the contents of all its subdirectories, recursively.
//  Then delete the file/directory of path itself. Symlinks are not followed (symlink is removed, not its target).
inline auto remove_all(const Path& path){
    return std::filesystem::remove_all(path.stdpath());
}

//  Copy a file.
inline bool copy_file(const Path& from, const Path& to){
    return std::filesystem::copy_file(from.stdpath(), to.stdpath());
}

//  Return the size of a file in bytes.
//  Throw std::filesystem::filesystem_error on underlying OS API errors
inline std::uintmax_t file_size(const Path& path){
    return std::filesystem::file_size(path.stdpath());
}

//  Return the size of a file in bytes.
//  If an error occurs, set `ec`. Execute `ec.clear()` if no errors occur.
inline std::uintmax_t file_size(const Path& path, std::error_code& ec){
    return std::filesystem::file_size(path.stdpath(), ec);
}

//  Rename a file or directory.
//  Throw std::filesystem::filesystem_error on underlying OS API errors
inline void rename(const Path& old_path, const Path& new_path){
    std::filesystem::rename(old_path.stdpath(), new_path.stdpath());
}

//  Rename a file or directory.
//  If an error occurs, set `ec`. Execute `ec.clear()` if no errors occur.
inline void rename(const Path& old_path, const Path& new_path, std::error_code& ec){
    std::filesystem::rename(old_path.stdpath(), new_path.stdpath(), ec);
}

inline Path current_path(){
    return std::filesystem::current_path();
}



}
}
#endif
