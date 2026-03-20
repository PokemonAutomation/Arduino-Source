/*  Resource Download Table
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
#include "ResourceDownloadTable.h"

#include <filesystem>
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

std::vector<DownloadedResource> deserialize_resource_list_json(const JsonValue& json){
    std::vector<DownloadedResource> resources;    

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

            DownloadedResource resource = {
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


std::vector<DownloadedResource> local_resource_download_list(){
	static std::vector<DownloadedResource> local_resources = deserialize_resource_list_json(load_json_file(RESOURCE_PATH() + "ResourceDownloadList.json"));

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

std::vector<DownloadedResource> remote_resource_download_list(){
	static std::vector<DownloadedResource> remote_resources = deserialize_resource_list_json(remote_resource_download_list_json());

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



std::vector<std::unique_ptr<ResourceDownloadRow>> get_resource_download_rows(){
    std::vector<std::unique_ptr<ResourceDownloadRow>> resource_rows;
    for (const DownloadedResource& resource : local_resource_download_list()){
        std::string resource_name = resource.resource_name;
        uint16_t expected_version_num = resource.version_num.value();
        std::optional<uint16_t> current_version_num;  // default nullopt

        Filesystem::Path filepath{DOWNLOADED_RESOURCE_PATH() + resource_name};
        bool is_downloaded = std::filesystem::is_directory(filepath);
        if (is_downloaded){
            current_version_num = get_resource_version_num(filepath);
        }

        ResourceVersionStatus version_status = get_version_status(expected_version_num, current_version_num);

        resource_rows.emplace_back(std::make_unique<ResourceDownloadRow>(std::move(resource_name), resource.size_decompressed_bytes, is_downloaded, current_version_num, version_status));
    }

    return resource_rows;
}




ResourceDownloadTable::~ResourceDownloadTable(){
    m_worker.wait_and_ignore_exceptions();
}

ResourceDownloadTable::ResourceDownloadTable()
    : StaticTableOption("<b>Resource Downloading:</b><br>Download resources not included in the initial download of the program.", LockMode::LOCK_WHILE_RUNNING, false)
    , m_resource_rows(get_resource_download_rows())
{
    add_resource_download_rows();

    finish_construction();
}
std::vector<std::string> ResourceDownloadTable::make_header() const{
    std::vector<std::string> ret{
        "Resource",
        "Size (MB)",
        "Downloaded",
        "Version",
        "",
        "",
    };
    return ret;
}

// UiWrapper ResourceDownloadTable::make_UiComponent(void* params) {
//     m_worker = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
//         [this]{ 
//             check_all_resource_versions(); 
//         }
//     );

//     return ConfigOptionImpl<StaticTableOption>::make_UiComponent(params);
// }

void ResourceDownloadTable::add_resource_download_rows(){
    for (auto& row_ptr : m_resource_rows){
        add_row(row_ptr.get());
    }
}




// void ResourceDownloadTable::check_all_resource_versions(){
//     std::vector<DownloadedResource> remote_resources = remote_resource_download_list();



//     // const JsonArray& resource_list = json_obj.get_array_throw("resourceList");

//     // test code
//     std::this_thread::sleep_for(std::chrono::seconds(5));

//     for (auto& row_ptr : m_resource_rows){
//         row_ptr->m_data->m_version_status_label.set_text("Hi");
//     }

// }



}
