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
    size_t file_size
)
    : StaticTableRow(resource_name)
    , m_resource_name(LockMode::LOCK_WHILE_RUNNING, resource_name)
    , m_file_size(file_size)
    , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(file_size))
    , m_download_button(*this)
    , m_delete_button(*this)
{
    PA_ADD_STATIC(m_resource_name);
    PA_ADD_STATIC(m_file_size_label);
    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
}

ResourceDownloadTable::ResourceDownloadTable()
    : StaticTableOption("<b>Resource Downloading:</b><br>Download resources not included in the initial download of the program.", LockMode::LOCK_WHILE_RUNNING, false)
{
    add_rows_from_resource_list_json();

    finish_construction();
}
std::vector<std::string> ResourceDownloadTable::make_header() const{
    std::vector<std::string> ret{
        "Resource",
        "Size",
        "",
        "",
    };
    return ret;
}


void ResourceDownloadTable::add_rows_from_resource_list_json(){
    JsonValue json = load_json_file(RESOURCE_PATH() + "ResourceList.json");

    try{
        const JsonObject& obj = json.to_object_throw();
        const JsonArray& resource_list = obj.get_array_throw("resourceList");
        for (const JsonValue& resource_val : resource_list){
            const JsonObject& resource_obj = resource_val.to_object_throw();

            std::string resource_name = resource_obj.get_string_throw("resourceName");
            int64_t decompressed_bytes = resource_obj.get_integer_throw("DecompressedBytes");

            add_row(std::make_unique<ResourceDownloadRow>(std::move(resource_name), decompressed_bytes));
        }

    }catch (ParseException& e){
        throw ParseException(e.message() + "\nJSON parsing error. Given JSON file doesn't match the expected format.");
    }

}




}
