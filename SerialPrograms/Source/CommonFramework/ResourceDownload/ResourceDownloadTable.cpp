/*  Resource Download Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "Common/Cpp/Exceptions.h"
// #include "CommonFramework/Logging/Logger.h"
// #include "CommonFramework/Tools/GlobalThreadPools.h"
// #include "CommonFramework/Tools/FileDownloader.h"
// #include "CommonFramework/Exceptions/OperationFailedException.h"
// #include "Common/Cpp/Json/JsonArray.h"
// #include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Filesystem.h"
#include "ResourceDownloadRow.h"
#include "ResourceDownloadTable.h"

// #include <filesystem>
// #include <thread>
// #include <unordered_set>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


std::vector<std::unique_ptr<ResourceDownloadRow>> get_resource_download_rows(){
    std::vector<std::unique_ptr<ResourceDownloadRow>> resource_rows;
    std::vector<DownloadedResourceMetadata> resource_list; 
    try{
        resource_list = local_resource_download_list();
    }catch(FileException&){
        return {};
    }
    
    for (const DownloadedResourceMetadata& resource : resource_list){
        std::string resource_name = resource.resource_name;
        uint16_t expected_version_num = resource.version_num.value();
        std::optional<uint16_t> current_version_num;  // default nullopt

        Filesystem::Path filepath{DOWNLOADED_RESOURCE_PATH() + resource_name};
        bool is_downloaded = std::filesystem::is_directory(filepath);
        if (is_downloaded){
            current_version_num = get_resource_version_num(filepath);
        }

        ResourceVersionStatus version_status = get_version_status(expected_version_num, current_version_num);

        resource_rows.emplace_back(std::make_unique<ResourceDownloadRow>(resource, is_downloaded, current_version_num, version_status));
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
        "Size",
        "Downloaded",
        "Version",
        "",
        "",
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
    for (std::unique_ptr<ResourceDownloadRow>& row_ptr : m_resource_rows){
        add_row(row_ptr.get());
    }
}




// void ResourceDownloadTable::check_all_resource_versions(){
//     std::vector<DownloadedResourceMetadata> remote_resources = remote_resource_download_list();



//     // const JsonArray& resource_list = json_obj.get_array_throw("resourceList");

//     // test code
//     std::this_thread::sleep_for(std::chrono::seconds(5));

//     for (auto& row_ptr : m_resource_rows){
//         row_ptr->m_data->m_version_status_label.set_text("Hi");
//     }

// }



}
