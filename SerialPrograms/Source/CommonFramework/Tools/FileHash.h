/*  File Hash
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FileHash_H
#define PokemonAutomation_FileHash_H

#include <string>
#include <functional>
#include "Common/Cpp/CancellableScope.h"


namespace PokemonAutomation{

// uses SHA 256
std::string hash_file(CancellableScope& scope, const std::string& file_path, std::function<void(uint64_t bytes_done, uint64_t total_bytes)> hash_progress);

}
#endif
