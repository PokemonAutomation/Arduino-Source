/*  File Downloader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FileDownloader_H
#define PokemonAutomation_FileDownloader_H

#include <string>

namespace PokemonAutomation{
namespace FileDownloader{
	std::string download_json_file(const std::string& url);
}
}
#endif
