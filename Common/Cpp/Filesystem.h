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

    //  Construct assuming UTF-8 encoding.
    Path(const char* path)
        : m_path(utf8_to_utf8(path))
    {}
    Path(const std::string& path)
        : m_path(utf8_to_utf8(path))
    {}

    //  Construct from UTF-8.
    Path(std::u8string path)
        : m_path(std::move(path))
    {}

    void clear(){
        m_path.clear();
    }


public:
    //  Convert to the C++ path so it can be passed to other library functions.
    operator const std::filesystem::path&() const{
        return m_path;
    }
    const std::filesystem::path& stdpath() const{
        return m_path;
    }


public:
    //  Return path string as UTF-8.
    std::string string() const{
        return utf8_to_str(m_path.u8string());
    }
    std::u8string u8string() const{
        return m_path.u8string();
    }

    Path filename() const{
        return m_path.filename();
    }
    Path parent_path() const{
        return m_path.parent_path();
    }
    Path stem() const{
        return m_path.stem();
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


public:
    friend Path operator/(const Path& x, const Path& y){
        return x.m_path / y.m_path;
    }
    friend std::ostream& operator<<(std::ostream& stream, const Path& x);


private:
    std::filesystem::path m_path;
};



inline bool exists(const Path& path){
    return std::filesystem::exists(path.stdpath());
}
inline bool create_directories(const Path& path){
    return std::filesystem::create_directories(path.stdpath());
}
inline auto remove_all(const Path& path){
    return std::filesystem::remove_all(path.stdpath());
}
inline bool copy_file(const Path& from, const Path& to){
    return std::filesystem::copy_file(from.stdpath(), to.stdpath());
}



}
}
#endif
