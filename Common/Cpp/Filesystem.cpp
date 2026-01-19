/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <filesystem>
#include "Common/Cpp/Strings/Unicode.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Filesystem.h"

namespace PokemonAutomation{
namespace Filesystem{


struct Path::Data{
    std::filesystem::path m_path;

//    template <class... Args>
//    Data(Args&&... args)
//        : m_path(std::forward<Args>(args)...)
//    {}

    Data() = default;
    Data(const char* path): m_path(path) {}
    Data(const std::string& path): m_path(path) {}
    Data(std::u8string path): m_path(std::move(path)) {}
    Data(std::filesystem::path path): m_path(std::move(path)) {}
};



Path::~Path() = default;
Path::Path(Path&&) = default;
Path& Path::operator=(Path&&) = default;
Path::Path(const Path&) = default;
Path& Path::operator=(const Path&) = default;


Path::Path()
    : m_data(CONSTRUCT_TOKEN)
{}
Path::Path(std::filesystem::path path)
    : m_data(CONSTRUCT_TOKEN, std::move(path))
{}
Path::Path(const char* path)
    : m_data(CONSTRUCT_TOKEN, utf8_to_utf8(path))
{}
Path::Path(const std::string& path)
    : m_data(CONSTRUCT_TOKEN, utf8_to_utf8(path))
{}
Path::Path(std::u8string path)
    : m_data(CONSTRUCT_TOKEN, std::move(path))
{}

void Path::clear(){
    m_data->m_path.clear();
}

Path::operator const std::filesystem::path&() const{
    return m_data->m_path;
}
const std::filesystem::path& Path::stdpath() const{
    return m_data->m_path;
}


std::string Path::string() const{
    return utf8_to_str(m_data->m_path.u8string());
}
std::u8string Path::u8string() const{
    return m_data->m_path.u8string();
}

Path Path::filename() const{
    return m_data->m_path.filename();
}
Path Path::parent_path() const{
    return m_data->m_path.parent_path();
}
Path Path::stem() const{
    return m_data->m_path.stem();
}


Path& Path::replace_extension(const Path& replacement){
    m_data->m_path.replace_extension(replacement);
    return *this;
}


bool operator==(const Path& x, const Path& y){
    return x.m_data->m_path == y.m_data->m_path;
}
Path operator/(const Path& x, const Path& y){
    return x.m_data->m_path / y.m_data->m_path;
}
std::ostream& operator<<(std::ostream& stream, const Path& x){
    return stream << x.m_data->m_path;
}


bool exists(const Path& path){
    return std::filesystem::exists(path.stdpath());
}
bool create_directories(const Path& path){
    return std::filesystem::create_directories(path.stdpath());
}
uintmax_t remove_all(const Path& path){
    return std::filesystem::remove_all(path.stdpath());
}
bool copy_file(const Path& from, const Path& to){
    return std::filesystem::copy_file(from.stdpath(), to.stdpath());
}


}
}
