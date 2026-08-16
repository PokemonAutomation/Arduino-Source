/*  File Hash
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ScopeExit.h"
#include "Common/Cpp/Containers/AlignedMalloc.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Cryptography/SHA256.h"
#include "FileHash.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



std::string hash_file(
    CancellableScope* scope,
    const std::string& file_path,
    std::function<void(uint64_t bytes_done, uint64_t total_bytes)> hash_progress
){
    Filesystem::Path path(file_path);
    FileIO file(path, FileMode::READ | FileMode::BINARY);
    if (!file.is_open()){
        throw InternalProgramError(
            nullptr,
            PA_CURRENT_FUNCTION,
            "hash_file: Could not open file."
        );
    }

    SHA256 hash;
    uint64_t file_size = Filesystem::file_size(path);
    uint64_t total_bytes_read = 0;

    constexpr size_t BUFFER_SIZE = 1024 * 1024;
    void* buffer = aligned_malloc(BUFFER_SIZE, 4096);   // Pre-allocate 1MB once
    ScopeExit sg([&]{ aligned_free(buffer); });

    size_t bytes_read;
    do{
        if (scope != nullptr){
            scope->throw_if_cancelled();
        }

        bytes_read = file.read(buffer, BUFFER_SIZE);
        hash.push(buffer, bytes_read);
        total_bytes_read += bytes_read;

        if (hash_progress != nullptr){
            hash_progress(total_bytes_read, file_size);
        }
    }while (bytes_read == BUFFER_SIZE);

    hash.finish();
    return hash.get_hash_hex();
}

}
