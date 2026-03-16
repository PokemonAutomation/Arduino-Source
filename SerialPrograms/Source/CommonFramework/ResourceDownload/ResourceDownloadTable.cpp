/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ResourceDownloadTable.h"

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
    bool is_downloaded,
    size_t file_size
)
    : StaticTableRow(resource_name)
    , m_resource_name(LockMode::LOCK_WHILE_RUNNING, resource_name)
    , m_is_downloaded(is_downloaded)
    , m_is_downloaded_label(LockMode::LOCK_WHILE_RUNNING, is_downloaded ? "Y" : "N")
    , m_file_size(file_size)
    , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(file_size))
    , m_download_button(*this)
    , m_delete_button(*this)
{
    PA_ADD_STATIC(m_resource_name);
    PA_ADD_STATIC(m_is_downloaded_label);
    PA_ADD_STATIC(m_file_size_label);
    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
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
        "Downloaded?",
        "Size (MB)",
        "",
        "",
    };
    return ret;
}

ResourceType get_resource_type_from_string(std::string type){
    if (type == "ZippedFolder"){
        return ResourceType::ZIP_FILE;
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "VideoFPS: Unknown enum.");                
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
        bool is_downloaded = false;
        std::string resource_name = resource.resource_name;
        resource_rows.emplace_back(std::make_unique<ResourceDownloadRow>(std::move(resource_name), is_downloaded, resource.size_decompressed_bytes));
    }

    return resource_rows;
}


void ResourceDownloadTable::add_resource_download_rows(){
    for (auto& row_ptr : m_resource_rows){
        add_row(row_ptr.get());
    }
}




}
