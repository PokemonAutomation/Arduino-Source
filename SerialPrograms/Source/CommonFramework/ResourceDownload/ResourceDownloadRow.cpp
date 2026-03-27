/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Options/LabelCellOption.h"
// #include "ResourceDownloadTable.h"
#include "ResourceDownloadRow.h"

// #include <thread>
// #include <fstream>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceDownloadRow
/////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string resource_version_to_string(ResourceVersionStatus version){
    switch(version){
    case ResourceVersionStatus::CURRENT:
        return "Current";
    case ResourceVersionStatus::OUTDATED:
        return "Outdated";
    case ResourceVersionStatus::NOT_APPLICABLE:
        return "--";
    // case ResourceVersionStatus::BLANK:
    //     return "";
    case ResourceVersionStatus::FUTURE_VERSION:
        return "Unsupported future version.<br>Please update the Computer Control program.";
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "resource_version_to_string: Unknown enum.");  
    }
}

struct ResourceDownloadRow::Data{
    Data(
        std::string&& resource_name,
        size_t file_size,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    )
        : m_resource_name(LockMode::LOCK_WHILE_RUNNING, resource_name)
        , m_file_size(file_size)
        , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(file_size))
        , m_is_downloaded(is_downloaded)
        , m_is_downloaded_label(LockMode::LOCK_WHILE_RUNNING, is_downloaded ? "Yes" : "--")
        , m_version_num(version_num)
        , m_version_status(version_status)
        , m_version_status_label(LockMode::LOCK_WHILE_RUNNING, resource_version_to_string(version_status))
    {}

    LabelCellOption m_resource_name;

    size_t m_file_size;
    LabelCellOption m_file_size_label;

    bool m_is_downloaded;
    LabelCellOption m_is_downloaded_label;

    std::optional<uint16_t> m_version_num;
    ResourceVersionStatus m_version_status;
    LabelCellOption m_version_status_label;


};


ResourceDownloadRow::~ResourceDownloadRow(){}
ResourceDownloadRow::ResourceDownloadRow(
    std::string&& resource_name,
    size_t file_size,
    bool is_downloaded,
    std::optional<uint16_t> version_num,
    ResourceVersionStatus version_status
)
    : StaticTableRow(resource_name)
    , m_data(CONSTRUCT_TOKEN, std::move(resource_name), file_size, is_downloaded, version_num, version_status)
    , m_download_button(*this)
    , m_delete_button(*this)
    , m_progress_bar(*this)
    , m_local_metadata(initialize_local_metadata())
{
    PA_ADD_STATIC(m_data->m_resource_name);
    PA_ADD_STATIC(m_data->m_file_size_label);
    PA_ADD_STATIC(m_data->m_is_downloaded_label);
    PA_ADD_STATIC(m_data->m_version_status_label);

    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
    PA_ADD_STATIC(m_progress_bar);
}



void ResourceDownloadRow::initialize_remote_metadata(){
    DownloadedResourceMetadata corresponding_remote_metadata;
    RemoteMetadataStatus status = RemoteMetadataStatus::NOT_AVAILABLE;
    std::vector<DownloadedResourceMetadata> all_remote_metadata = remote_resource_download_list();

    cout << "done remote_resource_download_list" << endl;
    
    std::string resource_name = m_data->m_resource_name.text();

    for (DownloadedResourceMetadata remote_metadata : all_remote_metadata){
        if (remote_metadata.resource_name == resource_name){
            corresponding_remote_metadata = remote_metadata;
            status = RemoteMetadataStatus::AVAILABLE;
            break;
        }
    }

    RemoteMetadata remote_metadata = {status, corresponding_remote_metadata};

    m_remote_metadata = std::make_unique<RemoteMetadata>(remote_metadata);
}

RemoteMetadata& ResourceDownloadRow::fetch_remote_metadata(){
    // Only runs once per instance
    std::call_once(init_flag, &ResourceDownloadRow::initialize_remote_metadata, this);
    return *m_remote_metadata;
}

DownloadedResourceMetadata ResourceDownloadRow::initialize_local_metadata(){
    DownloadedResourceMetadata corresponding_local_metadata;
    std::vector<DownloadedResourceMetadata> all_local_metadata = local_resource_download_list();
    
    std::string resource_name = m_data->m_resource_name.text();

    bool found = false;
    for (DownloadedResourceMetadata local_metadata : all_local_metadata){
        if (local_metadata.resource_name == resource_name){
            corresponding_local_metadata = local_metadata;
            found = true;
            break;
        }
    }

    if (!found){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "initialize_local_metadata: Corresponding DownloadedResourceMetadata not found in the local JSON file.");  
    }

    return corresponding_local_metadata;
}


void ResourceDownloadRow::run_download(DownloadedResourceMetadata resource_metadata){
    Logger& logger = global_logger_tagged();
    
    std::string url = resource_metadata.url;
    std::string resource_name = resource_metadata.resource_name;
    qint64 expected_size = resource_metadata.size_compressed_bytes;
    FileDownloader::download_file_to_disk(
        logger, 
        url, 
        DOWNLOADED_RESOURCE_PATH() + resource_name + "/temp.zip",
        expected_size,
        [this](int percentage_progress){
            download_progress(percentage_progress);
        }
    );
}


}
