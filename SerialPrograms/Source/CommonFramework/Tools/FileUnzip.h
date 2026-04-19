/*  File Unzip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FileUnzip_H
#define PokemonAutomation_FileUnzip_H

#include <string>
#include <functional>
#include "Common/Cpp/CancellableScope.h"


namespace PokemonAutomation{

// unzips the zip file located in zip_path, to target_dir
// if target_dir doesn't already exist, it will create it.
// throw OperationCancelledException if the CancellableScope is cancelled
// throw InternalProgramError if unzipping fails.
void unzip_file(
    CancellableScope& scope,
    const char* zip_path, 
    const char* target_dir, 
    std::function<void(uint64_t bytes_done, uint64_t total_bytes)> progress_callback
);

}
#endif
