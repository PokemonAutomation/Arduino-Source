/*  File Path
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

#ifndef PokemonAutomation_Filesystem_FilePath_H
#define PokemonAutomation_Filesystem_FilePath_H

#include <filesystem>
#include <iosfwd>

namespace PokemonAutomation{
namespace Filesystem{



class Path{
public:
    Path() = default;
    Path(std::filesystem::path path)
        : m_path(std::move(path))
    {}

    //  Construct assuming input path is UTF-8 encoding.
    Path(const char* path);
    //  Construct assuming input path is UTF-8 encoding.
    Path(const std::string& path);

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
    bool empty() const{
        return m_path.empty();
    }

    //  Return path string as UTF-8.
    std::string string() const;
    std::string string_slash_normalized() const;
    //  Return path string as UTF-8.
    std::u8string u8string() const{
        return m_path.u8string();
    }
    std::u8string u8string_slash_normalized() const;

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

    //  Connect two path components.
    Path& operator+=(const Path& x);
    friend Path operator+(const Path& x, const Path& y);

    //  Connect two path components with "/"
    Path& operator/=(const Path& x);
    friend Path operator/(const Path& x, const Path& y);

    friend std::ostream& operator<<(std::ostream& stream, const Path& x);

private:
    std::filesystem::path m_path;
};





}
}
#endif
