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

#ifndef PokemonAutomation_Filesystem_H
#define PokemonAutomation_Filesystem_H

#include <filesystem>
#include <iosfwd>
#include "Common/Cpp/Strings/Unicode.h"

namespace PokemonAutomation{
namespace Filesystem{


class Path{
public:
    Path() = default;
    Path(std::filesystem::path path)
        : m_path(std::move(path))
    {}

    //  Construct assuming input path is UTF-8 encoding.
    Path(const char* path)
        : m_path(utf8_to_utf8(path))
    {}
    //  Construct assuming input path is UTF-8 encoding.
    Path(const std::string& path)
        : m_path(utf8_to_utf8(path))
    {}

    //  Construct from UTF-8 string.
    Path(std::u8string path)
        : m_path(std::move(path))
    {}

    //  Clear path
    void clear(){
        m_path.clear();
    }

public:
    //  Implicit conversion to the C++ std::filesystem::path so it can be passed to other library functions.
    operator const std::filesystem::path&() const{
        return m_path;
    }
    //  Explicit conversion to the C++ std::filesystem::path so it can be passed to other library functions.
    const std::filesystem::path& stdpath() const{
        return m_path;
    }

public:
    //  Return path string as UTF-8.
    std::string string() const{
        return utf8_to_str(m_path.u8string());
    }
    //  Return path string as UTF-8.
    std::u8string u8string() const{
        return m_path.u8string();
    }

    //  Return the generic-format filename component of the path.
    Path filename() const{
        return m_path.filename();
    }
    //  Return the path to the parent directory.
    //  "/var/tmp/example.txt" -> "/var/tmp"
    //  "/var/tmp/." -> "/var/tmp"
    //  "/" -> "/"
    Path parent_path() const{
        return m_path.parent_path();
    }
    //  Return the filename identified by the generic-format path stripped of its extension.
    //  "/foo/bar.txt" -> "bar"
    //  "/foo/bar" -> "bar"
    //  "/foo/.bar" -> ".bar"
    //  "foo.bar.baz.tar" -> "foo.bar.baz"
    //  "/foo/." -> "."
    //  "/foo/.." -> ".."
    Path stem() const{
        return m_path.stem();
    }
    //  Return the extension of the filename component.
    //  "/foo/bar.txt" -> ".txt"
    //  "/foo/bar." -> "."
    //  "/foo/bar" -> ""
    //  "/foo/.bar" -> ""
    //  "/foo/..bar" -> ".bar"
    //  "/foo/." -> ""
    //  "/foo/.." -> ""
    Path extension() const{
        return m_path.extension();
    }

public:
    Path& replace_extension(const Path& replacement){
        m_path.replace_extension(replacement);
        return *this;
    }

public:
    friend bool operator==(const Path& x, const Path& y){
        return x.m_path == y.m_path;
    }

    //  Connect two path components with "/"
    friend Path operator/(const Path& x, const Path& y){
        return x.m_path / y.m_path;
    }
    friend std::ostream& operator<<(std::ostream& stream, const Path& x);

private:
    std::filesystem::path m_path;
};


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



}
}
#endif
