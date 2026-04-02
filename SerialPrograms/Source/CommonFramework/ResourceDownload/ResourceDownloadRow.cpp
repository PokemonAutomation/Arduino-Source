/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
// #include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Tools/FileUnzip.h"
#include "CommonFramework/Options/LabelCellOption.h"
// #include "ResourceDownloadTable.h"
#include "ResourceDownloadRow.h"

// #include <thread>
// #include <fstream>
#include <filesystem>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

    namespace fs = std::filesystem;

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

std::string is_downloaded_string(bool is_downloaded){
    return is_downloaded ? "Yes" : "--";
}

struct ResourceDownloadRow::Data{
    Data(
        std::string& resource_name,
        size_t file_size,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    )
        : m_resource_name(LockMode::LOCK_WHILE_RUNNING, resource_name)
        , m_file_size(file_size)
        , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(file_size))
        , m_is_downloaded(is_downloaded)
        , m_is_downloaded_label(LockMode::LOCK_WHILE_RUNNING, is_downloaded_string(is_downloaded))
        , m_version_num(version_num)
        , m_version_status(version_status)
        , m_version_status_label(LockMode::LOCK_WHILE_RUNNING, resource_version_to_string(version_status))
        , m_cancel_action(false)
    {}

    LabelCellOption m_resource_name;

    size_t m_file_size;
    LabelCellOption m_file_size_label;

    bool m_is_downloaded;
    LabelCellOption m_is_downloaded_label;

    std::optional<uint16_t> m_version_num;
    ResourceVersionStatus m_version_status;
    LabelCellOption m_version_status_label;

    std::atomic<bool> m_cancel_action;


};

void ResourceDownloadRow::set_version_status(ResourceVersionStatus version_status){
    m_data->m_version_status = version_status;
    m_data->m_version_status_label.set_text(resource_version_to_string(version_status));
}


void ResourceDownloadRow::set_is_downloaded(bool is_downloaded){
    m_data->m_is_downloaded = is_downloaded;
    m_data->m_is_downloaded_label.set_text(is_downloaded_string(is_downloaded));
}

void ResourceDownloadRow::set_cancel_action(bool cancel_action){
    m_data->m_cancel_action = cancel_action;
}


ResourceDownloadRow::~ResourceDownloadRow(){
    m_worker1.wait_and_ignore_exceptions();
    m_worker2.wait_and_ignore_exceptions();
    m_worker3.wait_and_ignore_exceptions();
}
ResourceDownloadRow::ResourceDownloadRow(
    DownloadedResourceMetadata local_metadata,
    bool is_downloaded,
    std::optional<uint16_t> version_num,
    ResourceVersionStatus version_status
)
    : StaticTableRow(local_metadata.resource_name)
    , m_local_metadata(local_metadata)
    , m_data(CONSTRUCT_TOKEN, local_metadata.resource_name, local_metadata.size_decompressed_bytes, is_downloaded, version_num, version_status)
    , m_download_button(*this)
    , m_delete_button(*this)
    , m_cancel_button(*this)
    , m_progress_bar(*this)
{
    PA_ADD_STATIC(m_data->m_resource_name);
    PA_ADD_STATIC(m_data->m_file_size_label);
    PA_ADD_STATIC(m_data->m_is_downloaded_label);
    PA_ADD_STATIC(m_data->m_version_status_label);

    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
    PA_ADD_STATIC(m_cancel_button);
    PA_ADD_STATIC(m_progress_bar);
}



void ResourceDownloadRow::initialize_remote_metadata(){
    DownloadedResourceMetadata corresponding_remote_metadata;
    RemoteMetadataStatus status = RemoteMetadataStatus::NOT_AVAILABLE;
    std::vector<DownloadedResourceMetadata> all_remote_metadata = remote_resource_download_list();

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

// DownloadedResourceMetadata ResourceDownloadRow::initialize_local_metadata(){
//     DownloadedResourceMetadata corresponding_local_metadata;
//     std::vector<DownloadedResourceMetadata> all_local_metadata = local_resource_download_list();
    
//     std::string resource_name = m_data->m_resource_name.text();

//     bool found = false;
//     for (DownloadedResourceMetadata local_metadata : all_local_metadata){
//         if (local_metadata.resource_name == resource_name){
//             corresponding_local_metadata = local_metadata;
//             found = true;
//             break;
//         }
//     }

//     if (!found){
//         throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "initialize_local_metadata: Corresponding DownloadedResourceMetadata not found in the local JSON file.");  
//     }

//     return corresponding_local_metadata;
// }

void ResourceDownloadRow::actions_done_reenable_buttons(){
    m_download_button.set_enabled(true);
    set_cancel_action(false);
}

void ResourceDownloadRow::ensure_remote_metadata_loaded(){
    m_worker1 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            std::string predownload_warning;
            RemoteMetadata& remote_handle = fetch_remote_metadata();
            // cout << "Fetched remote metadata" << endl;
            // throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "testing"); 

            predownload_warning = predownload_warning_summary(remote_handle);

            actions_done_reenable_buttons();
            emit metadata_fetch_finished(predownload_warning);

        }catch(OperationFailedException&){
            // cout << "failed" << endl;
            actions_done_reenable_buttons();
            emit download_failed();
            return;
        }catch(...){
            actions_done_reenable_buttons();
            // cout << "Exception thrown in thread" << endl;
            emit exception_caught("ResourceDownloadButton::ensure_remote_metadata_loaded");
            return;
        }
    
    }
    );

}

std::string ResourceDownloadRow::predownload_warning_summary(RemoteMetadata& remote_handle){

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

        std::string disk_space_requirement = "This will require " + std::to_string(decompressed_size + compressed_size) + " bytes of free space";

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



void ResourceDownloadRow::start_download(){
    m_worker2 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            
            // std::this_thread::sleep_for(std::chrono::seconds(7));
            RemoteMetadata& remote_handle = fetch_remote_metadata();
            if (remote_handle.status != RemoteMetadataStatus::AVAILABLE){
                switch (remote_handle.status){
                case RemoteMetadataStatus::UNINITIALIZED:
                    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "start_download: Remote metadata uninitialized.");
                case RemoteMetadataStatus::NOT_AVAILABLE:
                    cout << "start_download: Download not available. Cancel download." << endl;
                    throw OperationCancelledException();
                default:
                    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "start_download: Unknown enum."); 
                }
            }

            // Download is available
            DownloadedResourceMetadata metadata = remote_handle.metadata;
            run_download(metadata);

            cout << "Done Download" << endl;

            actions_done_reenable_buttons();
            emit download_finished();
        }catch(OperationCancelledException&){
            actions_done_reenable_buttons();                
            emit download_finished();
            return;
        }catch(OperationFailedException&){
            actions_done_reenable_buttons();
            emit download_failed();
            return;
        }catch(...){
            actions_done_reenable_buttons();
            emit exception_caught("ResourceDownloadButton::start_download");
            return;
        }
    }
    );

}


void ResourceDownloadRow::run_download(DownloadedResourceMetadata resource_metadata){
    Logger& logger = global_logger_tagged();
    // std::this_thread::sleep_for(std::chrono::seconds(5));

    std::string url = resource_metadata.url;
    std::string resource_name = resource_metadata.resource_name;
    qint64 expected_size = resource_metadata.size_compressed_bytes;

    std::string resource_directory = DOWNLOADED_RESOURCE_PATH() + resource_name;
    try{
        // delete directory and the old resource
        fs::remove_all(resource_directory);

        // download
        std::string zip_path = resource_directory + "/temp.zip";
        FileDownloader::download_file_to_disk(
            logger, 
            url, 
            zip_path,
            expected_size,
            [this](int percentage_progress){
                download_progress(percentage_progress);
            },
            [this](){
                return m_data->m_cancel_action.load();
            }
        );

        // unzip
        unzip_file(
            zip_path.c_str(), 
            resource_directory.c_str(),
            [this](int percentage_progress){
                unzip_progress(percentage_progress);
            }
            ,
            [this](){
                return m_data->m_cancel_action.load();
            }
        );

        // delete old zip file
        fs::remove(zip_path);

        // update the table labels
        set_is_downloaded(true);
        set_version_status(ResourceVersionStatus::CURRENT);
    }catch(OperationCancelledException& e){
        // delete directory and the resource
        fs::remove_all(resource_directory);

        // update the table labels
        set_is_downloaded(false);
        set_version_status(ResourceVersionStatus::NOT_APPLICABLE);

        throw e;
    }

}


void ResourceDownloadRow::start_delete(){
    m_worker3 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            std::string resource_name = m_local_metadata.resource_name;

            std::string resource_directory = DOWNLOADED_RESOURCE_PATH() + resource_name;
            // delete directory and the old resource
            fs::remove_all(resource_directory);

            // update the table labels
            set_is_downloaded(false);
            set_version_status(ResourceVersionStatus::NOT_APPLICABLE);
            
            // emit delete_finished();
        }catch(...){
            // actions_done_reenable_buttons();
            emit exception_caught("ResourceDownloadButton::start_delete");
            return;
        }
    }
    );

}


}
