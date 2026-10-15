/*  File Downloader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FileDownloader_H
#define PokemonAutomation_FileDownloader_H

#include <string>
#include <chrono>
#include <functional>
#include "Common/Cpp/CancellableScope.h"


namespace PokemonAutomation{
    class Logger;
    class JsonValue;
namespace FileDownloader{

//  Throws OperationFailedException if failed to download.
//  overall_timeout is a deadline for the whole request including the
//  connection phase (QNetworkRequest::setTransferTimeout() only limits
//  data transfer, so without it a connection that stalls would block
//  this call forever). std::chrono::seconds(0) (the default) imposes no
//  overall limit.
std::string download_file(Logger& logger, const std::string& url, std::chrono::seconds overall_timeout = std::chrono::seconds(0));

//  Throws OperationFailedException if failed to download.
void download_file_to_disk(
    CancellableScope& scope,
    Logger& logger, 
    const std::string& url, 
    const std::string& file_path, 
    uint64_t expected_size,
    std::function<void(uint64_t bytes_done, uint64_t total_bytes)> progress_callback
);

//  Throws OperationFailedException if failed to download.
//  Returns empty value if invalid JSON.
JsonValue download_json_file(Logger& logger, const std::string& url, std::chrono::seconds overall_timeout = std::chrono::seconds(0));


}
}
#endif
