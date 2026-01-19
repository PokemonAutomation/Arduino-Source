/*  File IO
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *  Cross-platform file I/O wrapper using C-style FILE* for better performance
 *  than std::ofstream while maintaining Unicode path support on all platforms.
 *
 *  On Windows, uses _wfopen() to handle Unicode paths correctly.
 *  On POSIX systems (macOS, Linux), uses fopen() with UTF-8 paths.
 */

#ifndef PokemonAutomation_FileIO_H
#define PokemonAutomation_FileIO_H

#include "Common/Cpp/Filesystem.h"
#include <cstdio>
#include <string>

namespace PokemonAutomation{


//  Open mode flags for FileIO.
//  Can be combined with bitwise OR.
enum class FileMode{
    READ        = 1 << 0,   //  Open for reading
    WRITE       = 1 << 1,   //  Open for writing (truncates existing file)
    APPEND      = 1 << 2,   //  Open for appending (writes go to end of file)
    BINARY      = 1 << 3,   //  Open in binary mode (no newline translation)
};

inline FileMode operator|(FileMode a, FileMode b){
    return static_cast<FileMode>(static_cast<int>(a) | static_cast<int>(b));
}
inline FileMode operator&(FileMode a, FileMode b){
    return static_cast<FileMode>(static_cast<int>(a) & static_cast<int>(b));
}
inline bool has_flag(FileMode mode, FileMode flag){
    return (static_cast<int>(mode) & static_cast<int>(flag)) != 0;
}


//  RAII wrapper around FILE* providing cross-platform Unicode path support
//  and better performance than std::ofstream.
//
//  Example usage:
//      FileIO file;
//      if (file.open(path, FileMode::APPEND | FileMode::BINARY)){
//          file.write(data.data(), data.size());
//          file.flush();
//      }
//
class FileIO{
public:
    FileIO() = default;
    ~FileIO();

    //  Construct and open a file with the given path and mode.
    //  Check is_open() to verify success.
    FileIO(const Filesystem::Path& path, FileMode mode);

    //  Non-copyable
    FileIO(const FileIO&) = delete;
    FileIO& operator=(const FileIO&) = delete;

    //  Movable
    FileIO(FileIO&& other) noexcept;
    FileIO& operator=(FileIO&& other) noexcept;

public:
    //  Open a file with the given path and mode.
    //  Returns true on success, false on failure.
    //  If a file is already open, it will be closed first.
    bool open(const Filesystem::Path& path, FileMode mode);

    //  Close the file. Safe to call multiple times.
    void close();

    //  Check if a file is currently open.
    bool is_open() const{
        return m_file != nullptr;
    }

public:
    //  Write data to the file.
    //  Returns the number of bytes written.
    size_t write(const void* data, size_t size);

    //  Write a string to the file.
    //  Returns the number of bytes written.
    size_t write(const std::string& str){
        return write(str.data(), str.size());
    }

    //  Read data from the file into buffer.
    //  Returns the number of bytes read.
    size_t read(void* buffer, size_t size);

    //  Flush buffered data to disk.
    //  Returns true on success.
    bool flush();

public:
    //  Get current file position.
    //  Returns -1 on error.
    int64_t tell() const;

    //  Seek to a position in the file.
    //  whence: SEEK_SET (beginning), SEEK_CUR (current), SEEK_END (end), defined in header <cstdio>.
    //  Returns true on success.
    bool seek(int64_t offset, int whence);

private:
    FILE* m_file = nullptr;
};


}
#endif
