/*  Resource Download Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/Logging/Logger.h"
// #include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Filesystem.h"
#include "ResourceDownloadHelpers.h"

// #include <filesystem>
// #include <thread>
// #include <unordered_set>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ResourceType get_resource_type_from_string(std::string type){
    if (type == "ZippedFolder"){
        return ResourceType::ZIP_FILE;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "get_resource_type_from_string: Unknown string.");
    }
    
}

std::vector<DownloadedResourceMetadata> deserialize_resource_list_json(const JsonValue& json){
    std::vector<DownloadedResourceMetadata> resources;    

    try{
        const JsonObject& obj = json.to_object_throw();
        const JsonArray& resource_list = obj.get_array_throw("resourceList");
        for (const JsonValue& resource_val : resource_list){
            const JsonObject& resource_obj = resource_val.to_object_throw();

            std::string resource_name = resource_obj.get_string_throw("resourceName");
            std::optional<uint16_t> version_num = (uint16_t)resource_obj.get_integer_throw("version");
            ResourceType resource_type = get_resource_type_from_string(resource_obj.get_string_throw("Type"));
            size_t compressed_bytes = (size_t)resource_obj.get_integer_throw("CompressedBytes");
            size_t decompressed_bytes = (size_t)resource_obj.get_integer_throw("DecompressedBytes");
            std::string url = resource_obj.get_string_throw("URL");

            DownloadedResourceMetadata resource = {
                resource_name,
                version_num,
                resource_type,
                compressed_bytes,
                decompressed_bytes,
                url
            };

            resources.emplace_back(std::move(resource));

        }

    }catch (ParseException& e){
        throw ParseException(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.");
    }

    return resources;
}


std::vector<DownloadedResourceMetadata> local_resource_download_list(){
	static std::vector<DownloadedResourceMetadata> local_resources = deserialize_resource_list_json(load_json_file(RESOURCE_PATH() + "ResourceDownloadList.json"));

	return local_resources;
}


JsonValue fetch_resource_download_list_json_from_remote(){
    Logger& logger = global_logger_tagged();
    JsonValue json;
    try{
        json = FileDownloader::download_json_file(
            logger,
            "https://raw.githubusercontent.com/jw098/Packages/refs/heads/download/Resources/ResourceDownloadList.json"
        );
    }catch (OperationFailedException&){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "fetch_resource_download_list_json_from_remote: Failed to download JSON.");  
    }
    
    return json;
}

const JsonValue& remote_resource_download_list_json(){
    static const JsonValue json = fetch_resource_download_list_json_from_remote();

    return json;
}

std::vector<DownloadedResourceMetadata> remote_resource_download_list(){
	static std::vector<DownloadedResourceMetadata> remote_resources = deserialize_resource_list_json(remote_resource_download_list_json());

	return remote_resources;
}

uint16_t get_resource_version_num(Filesystem::Path folder_path){
    std::string file_name = folder_path.string() + "/version.json";
    const JsonValue& json = load_json_file(file_name);

    const JsonObject& obj = json.to_object_throw();
    uint16_t version_num = (uint16_t)obj.get_integer_throw("version");

    return version_num;
}

ResourceVersionStatus get_version_status(uint16_t expected_version_num, std::optional<uint16_t> current_version_num){
    if (!current_version_num.has_value()){
        return ResourceVersionStatus::NOT_APPLICABLE;
    }

    if (current_version_num < expected_version_num){
        return ResourceVersionStatus::OUTDATED;
    }else if (current_version_num == expected_version_num){
        return ResourceVersionStatus::CURRENT;
    }else{ // current > expected
        return ResourceVersionStatus::FUTURE_VERSION;
    }
}



}
