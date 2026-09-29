/*  File Path
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Strings/Unicode.h"
#include "FilePath.h"

namespace PokemonAutomation{
namespace Filesystem{




Path::Path(const char* path)
    : m_path(utf8_to_utf8(path))
{}
Path::Path(const std::string& path)
    : m_path(utf8_to_utf8(path))
{}

std::string Path::string() const{
    return utf8_to_str(m_path.u8string());
}







std::string Path::string_slash_normalized() const{
    std::string ret = utf8_to_str(m_path.u8string());
#if _WIN32
    std::replace(ret.begin(), ret.end(), '\\', '/');
#endif
    return ret;
}
std::u8string Path::u8string_slash_normalized() const{
    std::u8string ret = m_path.u8string();
#if _WIN32
    std::replace(ret.begin(), ret.end(), '\\', '/');
#endif
    return ret;
}




Path& Path::operator+=(const Path& x){
    m_path += x.m_path;
    return *this;
}
Path operator+(const Path& x, const Path& y){
    std::filesystem::path ret = x;
    ret += y.m_path;
    return ret;
}
Path& Path::operator/=(const Path& x){
    m_path /= x.m_path;
    return *this;
}
Path operator/(const Path& x, const Path& y){
    return x.m_path / y.m_path;
}


std::ostream& operator<<(std::ostream& stream, const Path& x){
    return stream << x.m_path;
}



}
}
