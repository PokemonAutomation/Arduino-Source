/*  File Downloader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FileDownloader_H
#define PokemonAutomation_FileDownloader_H

#include <string>

namespace PokemonAutomation{
    class Logger;
    class JsonValue;
namespace FileDownloader{

//  Throws OperationFailedException if failed to download.
std::string download_file(Logger& logger, const std::string& url);

//  Throws OperationFailedException if failed to download.
void download_file_to_disk(Logger& logger, const std::string& url, const std::string& file_path);

//  Throws OperationFailedException if failed to download.
//  Returns empty value if invalid JSON.
JsonValue download_json_file(Logger& logger, const std::string& url);


}
}
#endif
