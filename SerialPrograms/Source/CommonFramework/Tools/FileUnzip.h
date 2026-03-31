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

void unzip_file(
    const char* zip_path, 
    const char* target_dir, 
    std::function<void(int)> progress_callback
);

}
#endif
