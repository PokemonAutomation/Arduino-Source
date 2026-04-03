/*  File Unzip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FileUnzip_H
#define PokemonAutomation_FileUnzip_H

#include <string>
#include <functional>

namespace PokemonAutomation{

// unzips the zip file located in zip_path, to target_dir
// if target_dir doesn't already exist, it will create it.
// throw OperationCancelledException if the is_cancelled callback returns true
// throw InternalProgramError if unzipping fails.
void unzip_file(
    const char* zip_path, 
    const char* target_dir, 
    std::function<void(int)> progress_callback,
    std::function<bool()> is_cancelled
);

}
#endif
