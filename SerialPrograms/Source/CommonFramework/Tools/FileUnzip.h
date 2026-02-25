/*  File Unzip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FileUnzip_H
#define PokemonAutomation_FileUnzip_H

#include <string>

namespace PokemonAutomation{

void unzip_file(const std::string& zip_path, const std::string& output_dir);

}
#endif
