/*  File Hash
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FileHash_H
#define PokemonAutomation_FileHash_H

#include <string>
#include <functional>

namespace PokemonAutomation{

// uses SHA 256
std::string hash_file(const std::string& file_path, std::function<void(int)> hash_progress);

}
#endif
