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
#include "ResourceDownload.h"
#include "SettingsResourceDownloadRow.h"
#include "SettingsResourceDownloadTable.h"

// #include <filesystem>
// #include <thread>
// #include <unordered_set>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


std::vector<std::unique_ptr<SettingsResourceDownloadRow>> SettingsResourceDownloadTable::get_resource_download_rows(){
    std::vector<std::unique_ptr<SettingsResourceDownloadRow>> resource_rows;
    std::vector<DownloadedResourceMetadata> resource_list; 
    try{
        resource_list = expected_resource_download_list();
    }catch(FileException&){
        return {};
    }
    
    for (uint16_t index = 0; index < resource_list.size(); index++){
        DownloadedResourceMetadata resource = resource_list[index];
        std::string resource_name = resource.resource_name;

        auto [is_downloaded, version_status, current_version_num] = get_local_version_info(resource_name);

        resource_rows.emplace_back(std::make_unique<SettingsResourceDownloadRow>(*this, resource_name, resource, is_downloaded, current_version_num, version_status));
    }

    return resource_rows;
}




SettingsResourceDownloadTable::~SettingsResourceDownloadTable(){
    // m_worker.wait_and_ignore_exceptions();
}

SettingsResourceDownloadTable::SettingsResourceDownloadTable()
    : StaticTableOption("<b>Resource Downloading:</b><br>Download resources not included in the initial download of the program.", LockMode::LOCK_WHILE_RUNNING, false)
    , m_resource_rows(get_resource_download_rows())
{
    add_resource_download_rows();

    finish_construction();
}
std::vector<std::string> SettingsResourceDownloadTable::make_header() const{
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


void SettingsResourceDownloadTable::add_resource_download_rows(){
    for (std::unique_ptr<SettingsResourceDownloadRow>& row_ptr : m_resource_rows){
        add_row(row_ptr.get());
    }
}

void SettingsResourceDownloadTable::connect_row_with_download(const std::string& resource_slug, std::shared_ptr<ResourceDownload>& download_ptr){

    for (const auto& row_ptr : m_resource_rows) {
        if (row_ptr->get_resource_slug() == resource_slug) {
            // download_ptr->add_listener(*row_ptr);
            row_ptr->connect_with_download(download_ptr);
            return; 
        }
    }

    // resource_slug not found within m_resource_rows
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "connect_row_with_download: resource_slug not found within m_resource_rows.");
}



}
