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


const std::vector<DownloadedResourceMetadata>& local_resource_download_list(){
    // cout << "local_resource_download_list" << endl;
    static std::vector<DownloadedResourceMetadata> local_resources = deserialize_resource_list_json(load_json_file(RESOURCE_PATH() + "ResourceDownloadList.json"));

    return local_resources;
}


JsonValue fetch_resource_download_list_json_from_remote(){
    Logger& logger = global_logger_tagged();
    JsonValue json = 
        FileDownloader::download_json_file(
            logger,
            "https://raw.githubusercontent.com/jw098/Packages/refs/heads/download/Resources/ResourceDownloadList.json"
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

ResourceVersionStatus get_version_status(DownloadedResourceMetadata& expected_resource){
    uint16_t expected_version_num = expected_resource.version_num.value();

    Filesystem::Path filepath{DOWNLOADED_RESOURCE_PATH() + expected_resource.resource_name};
    bool is_downloaded = std::filesystem::is_directory(filepath);
    std::optional<uint16_t> current_version_num;  // default nullopt
    if (is_downloaded){
        current_version_num = get_resource_version_num(filepath);
    }

    ResourceVersionStatus version_status = compare_version_num(expected_version_num, current_version_num);

    return version_status;
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


IndexedResourceMetadata get_resource_metadata_from_resource_type(const std::string& target_resource_type, const std::vector<DownloadedResourceMetadata>& resource_list){

    for (uint16_t index = 0; index < resource_list.size(); index++){
        const DownloadedResourceMetadata& metadata = resource_list[index];
        if (metadata.resource_name == target_resource_type){
            return IndexedResourceMetadata{metadata, index};
        }
    }

    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "get_resource_metadata_from_resource_type: Unable to find resource_type within resource_list."); 
}

bool is_resource_ready_in_queue(uint16_t max_concurrent_downloads, uint16_t resource_index, std::vector<uint16_t>& download_queue){

    // ASSUMES: the calling thread holds the m_lock. therefore, this function doesn't lock the mutex when accessing download_queue.
    // std::lock_guard<Mutex> lg(m_lock);  
    auto it = std::find(download_queue.begin(), download_queue.end(), resource_index);
    if (it == download_queue.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "is_download_ready_to_start: resource_index not found within download_queue.");
    }

    uint16_t download_position = (uint16_t)std::distance(download_queue.begin(), it);

    // cout << "download_position: " << std::to_string(download_position) << endl;

    return download_position < max_concurrent_downloads;
}


const std::unordered_set<std::string>& all_resource_names(){
    static std::unordered_set<std::string> names = [](){
        std::unordered_set<std::string> resource_names;
        for (const DownloadedResourceMetadata& resource : local_resource_download_list()){
            resource_names.insert(resource.resource_name);
        }
        return resource_names;
    }();

    return names;


}
