/*  Resource Download Helpers using Qt headers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "ResourceDownloadHelpersQt.h"


#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


JsonValue fetch_resource_download_list_json_from_remote(){
    Logger& logger = global_logger_tagged();
    JsonValue json = 
        FileDownloader::download_json_file(
            logger,
            "https://raw.githubusercontent.com/PokemonAutomation/Packages/refs/heads/master/Resources/ResourceDownloadList.json"
        );
    
    return json;
}


const JsonValue& remote_resource_download_list_json(){
    static const JsonValue json = fetch_resource_download_list_json_from_remote();

    return json;
}

const std::vector<DownloadedResourceMetadata>& remote_resource_download_list(){
    // cout << "remote_resource_download_list" << endl;
    static std::vector<DownloadedResourceMetadata> remote_resources = deserialize_resource_list_json(remote_resource_download_list_json());

    return remote_resources;
}

DownloadedResourceMetadata get_remote_resource_metadata_from_resource_slug(const std::string& target_resource_slug){
    Logger& logger = global_logger_tagged();
    std::vector<DownloadedResourceMetadata> remote_resources;

    // Step 1: Attempt to fetch the list of available downloads
    try{
        remote_resources = remote_resource_download_list();
    }catch(OperationFailedException&){
        std::cerr << "get_remote_resource_metadata_from_resource_slug: Error" << endl;
        throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
            "Error: Download failed. Failed to fetch the list of available downloads. Check your internet connection.");
    }

    // Step 2: Attempt to extract metadata for the specific slug
    try{
        return get_resource_metadata_from_resource_type(target_resource_slug, remote_resources);
    }catch(OperationFailedException&){
        std::cerr << "get_remote_resource_metadata_from_resource_slug: Error" << endl;
        throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
            "get_remote_resource_metadata_from_resource_slug: Unable to find " + target_resource_slug + " within resource_list. "
            "Likely caused by resource being no longer available for download. We recommend updating the Computer Control program.");
    }
}



}
