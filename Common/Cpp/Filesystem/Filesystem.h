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

#include <cstdint>
#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{


Path absolute(const Path& path);

//  Whether a path exists.
bool exists(const Path& path);

//  Create every missing directory along the path.
//  If all related directories already exist, return false. Otherwise return true.
bool create_directories(const Path& path);

//  Delete the contents of the path (if it is a directory) and the contents of all its subdirectories, recursively.
//  Then delete the file/directory of path itself. Symlinks are not followed (symlink is removed, not its target).
bool remove(const Path& path);
std::uintmax_t remove_all(const Path& path);

//  Copy a file.
bool copy_file(const Path& from, const Path& to);

//  Return the size of a file in bytes.
//  Throw std::filesystem::filesystem_error on underlying OS API errors
std::uintmax_t file_size(const Path& path);

//  Return the size of a file in bytes.
//  If an error occurs, set `ec`. Execute `ec.clear()` if no errors occur.
std::uintmax_t file_size(const Path& path, std::error_code& ec);

//  Rename a file or directory.
//  Throw std::filesystem::filesystem_error on underlying OS API errors
void rename(const Path& old_path, const Path& new_path);

//  Rename a file or directory.
//  If an error occurs, set `ec`. Execute `ec.clear()` if no errors occur.
void rename(const Path& old_path, const Path& new_path, std::error_code& ec);


//  The current working directory.
Path current_path();

//  The path to the actual binary that is running.
Path application_binary_name();
Path application_binary_path();
Path application_binary_directory();

//  Path to where the application is installed.
//  All immutable resources are relative to this.
Path application_install_path();

//  Path to where the application saves its output/settings.
Path application_scratch_path();


// Set a profile for program settings (/UserSettings/PROFILE_NAME/) on MacOS.
// Have to run the program with command-line argument "open -n PATH_TO_APP --args --profile PROFILE_NAME" to set the profile and launch a new window.
// This allows multiple instances of the program to run since settings are no longer shared.
#if defined(__APPLE__)
void set_startup_profile(int& argc, char* argv[]);
const std::string& STARTUP_PROFILE();
#endif



}
}
#endif
