/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "Common/Cpp/Exceptions.h"
#include "ResourceDownloadRow.h"
#include "ResourceDownloadOptions.h"

// #include <thread>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceDownloadButton
/////////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceDownloadButton::~ResourceDownloadButton(){
    m_worker1.wait_and_ignore_exceptions();
    m_worker2.wait_and_ignore_exceptions();
}


ResourceDownloadButton::ResourceDownloadButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDownloadButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
    , m_enabled(true)
{}


void ResourceDownloadButton::ensure_remote_metadata_loaded(){
    m_worker1 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
        [this]{ 
            try {
                m_enabled = false;
                // std::this_thread::sleep_for(std::chrono::seconds(1));
                std::string predownload_warning;
                RemoteMetadata& remote_handle = row.fetch_remote_metadata();
                // cout << "Fetched remote metadata" << endl;
                // throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "testing"); 

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

std::string ResourceDownloadButton::predownload_warning_summary(RemoteMetadata& remote_handle){

    std::string predownload_warning;

    switch (remote_handle.status){
    case RemoteMetadataStatus::UNINITIALIZED:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "predownload_warning_summary: Remote metadata uninitialized.");
    case RemoteMetadataStatus::NOT_AVAILABLE:
        predownload_warning = "Resource no longer available for download. We recommend updating the Computer Control program.";
        break;
    case RemoteMetadataStatus::AVAILABLE:
    {
        uint16_t local_version_num = row.m_local_metadata.version_num.value();

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



void ResourceDownloadButton::start_download(){
    m_worker2 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
        [this]{ 
            try {
                m_enabled = false;
                // std::this_thread::sleep_for(std::chrono::seconds(7));
                RemoteMetadata& remote_handle = row.fetch_remote_metadata();
                if (remote_handle.status != RemoteMetadataStatus::AVAILABLE){
                    switch (remote_handle.status){
                    case RemoteMetadataStatus::UNINITIALIZED:
                        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "start_download: Remote metadata uninitialized.");
                    case RemoteMetadataStatus::NOT_AVAILABLE:
                        cout << "start_download: Download not available. Cancel download." << endl;
                        m_enabled = true;
                        emit download_finished();
                        return;
                    default:
                        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "start_download: Unknown enum."); 
                    }
                }

                // Download is available
                DownloadedResourceMetadata metadata = remote_handle.metadata;
                row.run_download(metadata);

                cout << "Done Download" << endl;

                m_enabled = true;
                emit download_finished();
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceDeleteButton
/////////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceDeleteButton::ResourceDeleteButton(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceDeleteButton>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceProgressBar
/////////////////////////////////////////////////////////////////////////////////////////////////////////


ResourceProgressBar::ResourceProgressBar(ResourceDownloadRow& p_row)
    : ConfigOptionImpl<ResourceProgressBar>(LockMode::UNLOCK_WHILE_RUNNING)
    , row(p_row)
{}


}
