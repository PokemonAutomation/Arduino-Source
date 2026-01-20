/*  File IO
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstdio>
#include "FileIO.h"

#ifdef _WIN32
#include <stdio.h>   //  _wfopen, _ftelli64, _fseeki64
#endif

namespace PokemonAutomation{


//  Convert FileMode flags to C-style mode string.
//  Returns the appropriate mode string for fopen/wfopen.
static const char* mode_to_string(FileMode mode){
    bool read   = has_flag(mode, FileMode::READ);
    bool write  = has_flag(mode, FileMode::WRITE);
    bool append = has_flag(mode, FileMode::APPEND);
    bool binary = has_flag(mode, FileMode::BINARY);

    //  Determine base mode
    if (append){
        //  Append mode (with optional read)
        if (read){
            return binary ? "a+b" : "a+";
        }else{
            return binary ? "ab" : "a";
        }
    }else if (write){
        //  Write mode (truncate, with optional read)
        if (read){
            return binary ? "w+b" : "w+";
        }else{
            return binary ? "wb" : "w";
        }
    }else if (read){
        //  Read-only mode
        return binary ? "rb" : "r";
    }

    //  Default to read binary if no mode specified
    return "rb";
}

#ifdef _WIN32
//  Convert FileMode flags to wide C-style mode string for Windows.
static const wchar_t* mode_to_wstring(FileMode mode){
    bool read   = has_flag(mode, FileMode::READ);
    bool write  = has_flag(mode, FileMode::WRITE);
    bool append = has_flag(mode, FileMode::APPEND);
    bool binary = has_flag(mode, FileMode::BINARY);

    if (append){
        if (read){
            return binary ? L"a+b" : L"a+";
        }else{
            return binary ? L"ab" : L"a";
        }
    }else if (write){
        if (read){
            return binary ? L"w+b" : L"w+";
        }else{
            return binary ? L"wb" : L"w";
        }
    }else if (read){
        return binary ? L"rb" : L"r";
    }

    return L"rb";
}
#endif


FileIO::~FileIO(){
    close();
}

FileIO::FileIO(const Filesystem::Path& path, FileMode mode)
    : m_file(nullptr)
{
    open(path, mode);
}

FileIO::FileIO(FileIO&& other) noexcept
    : m_file(other.m_file)
{
    other.m_file = nullptr;
}

FileIO& FileIO::operator=(FileIO&& other) noexcept{
    if (this != &other){
        close();
        m_file = other.m_file;
        other.m_file = nullptr;
    }
    return *this;
}

bool FileIO::open(const Filesystem::Path& path, FileMode mode){
    //  Close existing file if open
    close();

#ifdef _WIN32
    //  Windows: Use _wfopen for Unicode path support.
    //  std::filesystem::path::c_str() returns wchar_t* on Windows.
    const wchar_t* wmode = mode_to_wstring(mode);
    m_file = _wfopen(path.stdpath().c_str(), wmode);
#else
    //  POSIX (macOS, Linux): Use fopen with UTF-8 path.
    //  These systems natively support UTF-8 in file paths.
    const char* cmode = mode_to_string(mode);
    m_file = fopen(path.string().c_str(), cmode);
#endif

    return m_file != nullptr;
}

void FileIO::close(){
    if (m_file){
        fclose((FILE*)m_file);
        m_file = nullptr;
    }
}

size_t FileIO::write(const void* data, size_t size){
    if (!m_file || size == 0){
        return 0;
    }
    return fwrite(data, 1, size, (FILE*)m_file);
}

size_t FileIO::read(void* buffer, size_t size){
    if (!m_file || size == 0){
        return 0;
    }
    return fread(buffer, 1, size, (FILE*)m_file);
}

bool FileIO::flush(){
    if (!m_file){
        return false;
    }
    return fflush((FILE*)m_file) == 0;
}

int64_t FileIO::tell() const{
    if (!m_file){
        return -1;
    }
#ifdef _WIN32
    //  Windows: Use _ftelli64() for 64-bit file positions.
    //  ftell() returns long which is 32-bit on Windows, limiting to 2GB files.
    return _ftelli64((FILE*)m_file);
#else
    //  POSIX: Use ftello() which returns off_t (64-bit on 64-bit systems).
    return static_cast<int64_t>(ftello((FILE*)m_file));
#endif
}

bool FileIO::seek(int64_t offset, int whence){
    if (!m_file){
        return false;
    }
#ifdef _WIN32
    //  Windows: Use _fseeki64() for 64-bit file positions.
    return _fseeki64((FILE*)m_file, offset, whence) == 0;
#else
    //  POSIX: Use fseeko() which accepts off_t (64-bit on 64-bit systems).
    return fseeko((FILE*)m_file, static_cast<off_t>(offset), whence) == 0;
#endif
}


}
