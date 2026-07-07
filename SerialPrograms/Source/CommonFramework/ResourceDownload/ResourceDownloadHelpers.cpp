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


std::vector<DownloadedResourceMetadata> deserialize_resource_list_json(const JsonValue& json){
    std::vector<DownloadedResourceMetadata> resources;    

    try{
        const JsonObject& obj = json.to_object_throw();
        const JsonArray& resource_list = obj.get_array_throw("ResourceList");
        for (const JsonValue& resource_val : resource_list){
            const JsonObject& resource_obj = resource_val.to_object_throw();

            std::string resource_name = resource_obj.get_string_throw("ResourceName");
            std::optional<uint16_t> version_num = (uint16_t)resource_obj.get_integer_throw("Version");
            size_t compressed_bytes = (size_t)resource_obj.get_integer_throw("CompressedBytes");
            size_t decompressed_bytes = (size_t)resource_obj.get_integer_throw("DecompressedBytes");
            std::string url = resource_obj.get_string_throw("URL");
            std::string sha256 = resource_obj.get_string_throw("SHA256");

            DownloadedResourceMetadata resource = {
                resource_name,
                version_num,
                compressed_bytes,
                decompressed_bytes,
                url,
                sha256
            };

            resources.emplace_back(std::move(resource));

        }

    }catch (ParseException&){
        std::cerr << "JSON parsing error. Given JSON file doesn't match the expected format." << endl;
        // throw ParseException("JSON parsing error. Given JSON file doesn't match the expected format.");
        return std::vector<DownloadedResourceMetadata>();
    }

    return resources;
}


const std::vector<DownloadedResourceMetadata>& expected_resource_download_list(){
    // cout << "expected_resource_download_list" << endl;
    static std::vector<DownloadedResourceMetadata> expected_resources = deserialize_resource_list_json(
        load_json_file(RESOURCE_PATH() + "ResourceDownloadList.json")
    );

    return expected_resources;
}


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

std::optional<uint16_t> get_resource_version_num(Filesystem::Path folder_path){
    try{
        std::string file_name = folder_path.string() + "/version.json";
        const JsonValue& json = load_json_file(file_name);

        const JsonObject& obj = json.to_object_throw();
        uint16_t version_num = (uint16_t)obj.get_integer_throw("version");
        return version_num;
    }catch(...){
        std::cerr << "Unable to determine the version number from version.json." << endl;
        return std::nullopt;
    }

}

ResourceVersionStatus compare_version_num(uint16_t expected_version_num, std::optional<uint16_t> current_version_num){
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

ResourceVersionInfo get_local_version_info(const std::string& target_resource_slug){
    DownloadedResourceMetadata expected_resource = get_expected_resource_metadata_from_resource_slug(target_resource_slug);
    uint16_t expected_version_num = expected_resource.version_num.value();

    Filesystem::Path filepath{DOWNLOADED_RESOURCE_PATH() + expected_resource.resource_name};
    bool is_downloaded = std::filesystem::is_directory(filepath);
    std::optional<uint16_t> current_version_num;  // default nullopt
    if (is_downloaded){
        current_version_num = get_resource_version_num(filepath);
    }

    ResourceVersionStatus version_status = compare_version_num(expected_version_num, current_version_num);

    return ResourceVersionInfo{
        .is_downloaded = is_downloaded,
        .version_status = version_status,
        .current_version_num = current_version_num
    };
}


DownloadedResourceMetadata get_resource_metadata_from_resource_type(const std::string& target_resource_slug, const std::vector<DownloadedResourceMetadata>& resource_list){

    for (uint16_t index = 0; index < resource_list.size(); index++){
        const DownloadedResourceMetadata& metadata = resource_list[index];
        if (metadata.resource_name == target_resource_slug){
            return metadata;
        }
    }

    Logger& logger = global_logger_tagged();
    throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
        "get_resource_metadata_from_resource_type: Unable to find target_resource_slug within resource_list.");
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

DownloadedResourceMetadata get_expected_resource_metadata_from_resource_slug(const std::string& target_resource_slug){
    try{
        return get_resource_metadata_from_resource_type(target_resource_slug, expected_resource_download_list());
    }catch(OperationFailedException&){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "get_expected_resource_metadata_from_resource_slug: Unable to find target_resource_slug within resource_list. This shouldn't happen."); 
    }
}


const std::unordered_set<std::string>& all_resource_names(){
    static std::unordered_set<std::string> names = [](){
        std::unordered_set<std::string> resource_names;
        for (const DownloadedResourceMetadata& resource : expected_resource_download_list()){
            resource_names.insert(resource.resource_name);
        }
        return resource_names;
    }();

    return names;


}

}
