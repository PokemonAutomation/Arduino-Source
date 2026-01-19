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
 *  Since we are wrapping "std::filesytem", we also PIMPL it to reduce
 *  compilation times.
 *
 */

#ifndef PokemonAutomation_Filesystem_H
#define PokemonAutomation_Filesystem_H

#include <string>
#include "Common/Cpp/Containers/Pimpl.h"

namespace std::filesystem{
    class path;
}

namespace PokemonAutomation{
namespace Filesystem{


class Path{
public:
    ~Path();
    Path(Path&&);
    Path& operator=(Path&&);
    Path(const Path&);
    Path& operator=(const Path&);


public:
    Path();
    Path(std::filesystem::path path);

    //  Construct assuming UTF-8 encoding.
    Path(const char* path);
    Path(const std::string& path);

    //  Construct from UTF-8.
    Path(std::u8string path);

    void clear();


public:
    //  Convert to the C++ path so it can be passed to other library functions.
    operator const std::filesystem::path&() const;
    const std::filesystem::path& stdpath() const;


public:
    //  Return path string as UTF-8.
    std::string string() const;
    std::u8string u8string() const;

    Path filename() const;
    Path parent_path() const;
    Path stem() const;


public:
    Path& replace_extension(const Path& replacement);


public:
    friend bool operator==(const Path& x, const Path& y);


public:
    friend Path operator/(const Path& x, const Path& y);
    friend std::ostream& operator<<(std::ostream& stream, const Path& x);


private:
    struct Data;
    Pimpl<Data> m_data;
};



bool exists(const Path& path);
bool create_directories(const Path& path);
uintmax_t remove_all(const Path& path);
bool copy_file(const Path& from, const Path& to);



}
}
#endif
