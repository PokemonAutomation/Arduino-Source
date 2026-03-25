/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QMessageBox>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/LabelCellOption.h"
// #include "ResourceDownloadTable.h"
#include "ResourceDownloadRow.h"

#include <thread>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

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


ResourceDownloadButton::~ResourceDownloadButton(){
    m_worker1.wait_and_ignore_exceptions();
    m_worker2.wait_and_ignore_exceptions();
}


ResourceDownloadButton::ResourceDownloadButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
    , m_local_metadata(get_local_metadata())
{}


void ResourceDownloadButton::ensure_remote_metadata_loaded(){
    m_worker1 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
        [this]{ 
            try {
                m_enabled = false;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::string predownload_warning;
                ResourceDownloadButton::RemoteMetadata& remote_handle = fetch_remote_metadata();
                // cout << "Fetched remote metadata" << endl;

                predownload_warning = predownload_warning_summary(remote_handle);
                m_enabled = true;
                emit metadata_fetch_finished(predownload_warning);

            }catch(...){
                m_enabled = true;
                // cout << "Exception thrown in thread" << endl;
                emit exception_caught("ResourceDownloadButton::ensure_remote_metadata_loaded");
                // std::cerr << "Error: Unknown error. Embedding session failed." << std::endl;
                // QMessageBox box;
                // box.warning(nullptr, "Error:",
                //     QString::fromStdString("Error: Unknown error. Embedding session failed."));
                return;
            }
        
        }
    );

}

std::string ResourceDownloadButton::predownload_warning_summary(ResourceDownloadButton::RemoteMetadata& remote_handle){

    std::string predownload_warning;

    switch (remote_handle.status){
    case RemoteMetadataStatus::UNINITIALIZED:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "predownload_warning_summary: Remote metadata uninitialized.");
    case RemoteMetadataStatus::NOT_AVAILABLE:
        predownload_warning = "Resource no longer available for download. We recommend updating the Computer Control program.";
        break;
    case RemoteMetadataStatus::AVAILABLE:
    {
        uint16_t local_version_num = m_local_metadata.version_num.value();

        DownloadedResourceMetadata remote_metadata = remote_handle.metadata;
        uint16_t remote_version_num = remote_metadata.version_num.value();
        size_t compressed_size = remote_metadata.size_compressed_bytes;
        size_t decompressed_size = remote_metadata.size_decompressed_bytes;

        std::string disk_space_requirement = "";

        if (decompressed_size > 100000){
            disk_space_requirement = "This will require " + std::to_string(decompressed_size + compressed_size) + " bytes of free space";
        }else{
            disk_space_requirement = "This will require " + std::to_string(decompressed_size) + " bytes of free space";
        }

        if (local_version_num < remote_version_num){
            predownload_warning = "The resource you are downloading is a more updated version than the program expects. "
            "This may or may not cause issues with the programs. "
            "We recommend updating the Computer Control program.<br>" +
            disk_space_requirement;
        }else if (local_version_num == remote_version_num){
            predownload_warning = "Update available.<br>" + disk_space_requirement;
        }else if (local_version_num > remote_version_num){
            predownload_warning = "The resource you are downloading is a less updated version than the program expects. "
            "Please report this as a bug.<br>" +
            disk_space_requirement;
        }
    }
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "predownload_warning_summary: Unknown enum."); 
    }

    return predownload_warning;
}


void ResourceDownloadButton::initialize_remote_metadata(){
    DownloadedResourceMetadata corresponding_remote_metadata;
    RemoteMetadataStatus status = RemoteMetadataStatus::NOT_AVAILABLE;
    std::vector<DownloadedResourceMetadata> all_remote_metadata = remote_resource_download_list();

    cout << "done remote_resource_download_list" << endl;
    
    std::string resource_name = row.m_data->m_resource_name.text();

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

ResourceDownloadButton::RemoteMetadata& ResourceDownloadButton::fetch_remote_metadata(){
    // Only runs once per instance
    std::call_once(init_flag, &ResourceDownloadButton::initialize_remote_metadata, this);
    return *m_remote_metadata;
}

DownloadedResourceMetadata ResourceDownloadButton::get_local_metadata(){
    DownloadedResourceMetadata corresponding_local_metadata;
    std::vector<DownloadedResourceMetadata> all_local_metadata = local_resource_download_list();
    
    std::string resource_name = row.m_data->m_resource_name.text();

    bool found = false;
    for (DownloadedResourceMetadata local_metadata : all_local_metadata){
        if (local_metadata.resource_name == resource_name){
            corresponding_local_metadata = local_metadata;
            found = true;
            break;
        }
    }

    if (!found){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "get_local_metadata: Corresponding DownloadedResourceMetadata not found in the local JSON file.");  
    }

    return corresponding_local_metadata;
}


void ResourceDownloadButton::run_download(){
    m_worker2 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
        [this]{ 
            m_enabled = false;
            std::this_thread::sleep_for(std::chrono::seconds(7));
            cout << "Done Download" << endl;
            // show_update_box("Download", "Download", "Do you want to download?");

            m_enabled = true;
            emit download_finished();
        }
    );

}

ResourceDeleteButton::ResourceDeleteButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDeleteButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}



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
{
    PA_ADD_STATIC(m_data->m_resource_name);
    PA_ADD_STATIC(m_data->m_file_size_label);
    PA_ADD_STATIC(m_data->m_is_downloaded_label);
    PA_ADD_STATIC(m_data->m_version_status_label);

    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
}


}
