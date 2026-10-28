/*  Filesystem
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
#include "CompileTimeBackends.h"
#include "FilePath.h"
#include "Filesystem.h"

#ifdef _WIN32
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

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

bool remove(const Path& path){
    return std::filesystem::remove(path);
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

void set_current_path(const Path& path){
    std::filesystem::current_path(path.stdpath());
}
void set_current_path(const Path& path, std::error_code& ec){
    std::filesystem::current_path(path.stdpath(), ec);
}

Path relative(const Path& path, std::error_code& ec){
    std::error_code local_ec;
    std::filesystem::path ret = std::filesystem::relative(path.stdpath(), local_ec);
    if (local_ec){
        ec = local_ec;
        std::filesystem::path abs = std::filesystem::absolute(path.stdpath(), ec);
        if (!ec){
            ret = abs;
        }
    }else{
        ec.clear();
    }
    return ret;
}

bool is_directory(const Path& path){
    std::error_code ec;
    return std::filesystem::is_directory(path.stdpath(), ec);
}
bool is_regular_file(const Path& path){
    std::error_code ec;
    return std::filesystem::is_regular_file(path.stdpath(), ec);
}


std::vector<Path> list_directory(const Path& path, bool recursive){
    std::vector<Path> ret;
    std::error_code ec;
    auto add_entry = [](std::vector<Path>& ret, const std::filesystem::path& p){
        const std::string name = p.filename().string();
        if (!name.empty() && name.front() != '.'){
            ret.emplace_back(Path(p));
        }
    };
    if (recursive){
        std::filesystem::recursive_directory_iterator it(
            path.stdpath(),
            std::filesystem::directory_options::skip_permission_denied,
            ec
        );
        if (ec){
            return ret;
        }
        std::filesystem::recursive_directory_iterator end;
        for (;;){
            if (it == end){
                break;
            }
            add_entry(ret, it->path());
            it.increment(ec);
            if (ec){
                break;
            }
        }
    }else{
        std::filesystem::directory_iterator it(
            path.stdpath(),
            std::filesystem::directory_options::skip_permission_denied,
            ec
        );
        if (ec){
            return ret;
        }
        std::filesystem::directory_iterator end;
        for (;;){
            if (it == end){
                break;
            }
            add_entry(ret, it->path());
            it.increment(ec);
            if (ec){
                break;
            }
        }
    }
    //  Sort by name, case-insensitively (matches Qt's QDir::Name ordering).
    auto lower_name = [](const Path& p){
        std::string name = p.filename().string();
        std::transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c){ return (char)std::tolower(c); }
        );
        return name;
    };
    std::sort(ret.begin(), ret.end(), [&lower_name](const Path& a, const Path& b){
        return lower_name(a) < lower_name(b);
    });
    return ret;
}

#ifdef _WIN32
bool is_readable(const Path& path){
    return _waccess(path.stdpath().c_str(), 4) == 0;
}
bool is_writable(const Path& path){
    return _waccess(path.stdpath().c_str(), 2) == 0;
}
#else
bool is_readable(const Path& path){
    return ::access(path.string().c_str(), R_OK) == 0;
}
bool is_writable(const Path& path){
    return ::access(path.string().c_str(), W_OK) == 0;
}
#endif


Path unique_temp_path(const Path& path){
    std::string suffix = ".tmp-";
#ifdef _WIN32
    suffix += std::to_string((unsigned int)_getpid());
#else
    suffix += std::to_string((unsigned int)getpid());
#endif
    unsigned int rand = std::random_device{}();
    char rand_str[16];
    std::snprintf(rand_str, sizeof(rand_str), "-%08x", rand);
    suffix += rand_str;
    return path.parent_path() / Path(path.filename().string() + suffix);
}

Path application_binary_name(){
    return application_binary_path().filename();
}
Path application_binary_directory(){
    return application_binary_path().parent_path();
}




}
}
