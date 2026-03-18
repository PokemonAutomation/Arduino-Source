/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Filesystem.h"
#include "ResourceDownloadTable.h"

#include <filesystem>
#include <thread>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

ResourceDownloadButton::ResourceDownloadButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , option(p_row)
{}

ResourceDeleteButton::ResourceDeleteButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDeleteButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , option(p_row)
{}


ResourceDownloadRow::ResourceDownloadRow(
    std::string&& resource_name,
    size_t file_size,
    bool is_downloaded,
    ResourceVersion version
)
    : StaticTableRow(resource_name)
    , m_resource_name(LockMode::LOCK_WHILE_RUNNING, resource_name)
    , m_file_size(file_size)
    , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(file_size))
    , m_is_downloaded(is_downloaded)
    , m_is_downloaded_label(LockMode::LOCK_WHILE_RUNNING, is_downloaded ? "Yes" : "--")
    , m_version(version)
    , m_version_label(LockMode::LOCK_WHILE_RUNNING, resource_version_to_string(version))
    , m_download_button(*this)
    , m_delete_button(*this)
{
    PA_ADD_STATIC(m_resource_name);
    PA_ADD_STATIC(m_file_size_label);
    PA_ADD_STATIC(m_is_downloaded_label);
    PA_ADD_STATIC(m_version_label);

    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
}

std::string ResourceDownloadRow::resource_version_to_string(ResourceVersion version){
    switch(version){
    case ResourceVersion::CURRENT:
        return "Current";
    case ResourceVersion::OUTDATED:
        return "Outdated";
    case ResourceVersion::NOT_APPLICABLE:
        return "--";
    case ResourceVersion::BLANK:
        return "";
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "resource_version_to_string: Unknown enum.");  
    }
}

ResourceDownloadTable::~ResourceDownloadTable(){
    m_worker.wait_and_ignore_exceptions();
}

ResourceDownloadTable::ResourceDownloadTable()
    : StaticTableOption("<b>Resource Downloading:</b><br>Download resources not included in the initial download of the program.", LockMode::LOCK_WHILE_RUNNING, false)
    , m_resources(deserialize_resource_list_json())
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

UiWrapper ResourceDownloadTable::make_UiComponent(void* params) {
    m_worker = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
        [this]{ 
            check_all_resource_versions(); 
        }
    );

    return ConfigOptionImpl<StaticTableOption>::make_UiComponent(params);
}

ResourceType get_resource_type_from_string(std::string type){
    if (type == "ZippedFolder"){
        return ResourceType::ZIP_FILE;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "get_resource_type_from_string: Unknown enum.");                
    }
    
}

std::vector<DownloadedResource> ResourceDownloadTable::deserialize_resource_list_json(){
    std::vector<DownloadedResource> resources;
    JsonValue json = load_json_file(RESOURCE_PATH() + "ResourceList.json");

    try{
        const JsonObject& obj = json.to_object_throw();
        const JsonArray& resource_list = obj.get_array_throw("resourceList");
        for (const JsonValue& resource_val : resource_list){
            const JsonObject& resource_obj = resource_val.to_object_throw();

            std::string resource_name = resource_obj.get_string_throw("resourceName");
            ResourceType resource_type = get_resource_type_from_string(resource_obj.get_string_throw("Type"));
            size_t compressed_bytes = (size_t)resource_obj.get_integer_throw("CompressedBytes");
            size_t decompressed_bytes = (size_t)resource_obj.get_integer_throw("DecompressedBytes");
            std::string url = resource_obj.get_string_throw("URL");

            DownloadedResource resource = {
                resource_name,
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

std::vector<std::unique_ptr<ResourceDownloadRow>> ResourceDownloadTable::get_resource_download_rows(){
    std::vector<std::unique_ptr<ResourceDownloadRow>> resource_rows;
    for (const DownloadedResource& resource : m_resources){
        std::string resource_name = resource.resource_name;
        ResourceVersion version = ResourceVersion::BLANK;

        Filesystem::Path filepath{DOWNLOADED_RESOURCE_PATH() + resource_name};
        bool is_downloaded = std::filesystem::is_directory(filepath);

        resource_rows.emplace_back(std::make_unique<ResourceDownloadRow>(std::move(resource_name), resource.size_decompressed_bytes, is_downloaded, version));
    }

    return resource_rows;
}


void ResourceDownloadTable::add_resource_download_rows(){
    for (auto& row_ptr : m_resource_rows){
        add_row(row_ptr.get());
    }
}

void ResourceDownloadTable::check_all_resource_versions(){

    // test code
    std::this_thread::sleep_for(std::chrono::seconds(5));

    for (auto& row_ptr : m_resource_rows){
        row_ptr->m_version_label.set_text("Hi");
    }

}



}
