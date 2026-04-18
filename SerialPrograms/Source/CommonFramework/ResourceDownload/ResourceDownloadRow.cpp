/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/ListenerSet.h"
// #include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Tools/FileUnzip.h"
#include "CommonFramework/Tools/FileHash.h"
#include "Common/Cpp/Filesystem.h"
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
// DownloadThread
/////////////////////////////////////////////////////////////////////////////////////////////////////////

DownloadThread::~DownloadThread(){
    // cout << "~DownloadThread" << endl;
    this->cancel();
    m_worker.wait_and_ignore_exceptions();
}
DownloadThread::DownloadThread(ConstructorKey, ResourceDownloadRow& row) 
    : CancellableScope()
    , m_row(row)
{}

std::shared_ptr<DownloadThread> DownloadThread::create(ResourceDownloadRow& row){
    return std::make_shared<DownloadThread>(ConstructorKey{}, row);
}

void DownloadThread::start_download_thread(){
    auto self = shared_from_this();
    m_worker = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this, self]{
        
        // runs when lambda is finished
        // updates button state, and releases DownloadRow's ownership over this thread
        // the thread cleans itself up when `self` goes out of scope at the end of this lambda
        struct ScopeGuard {
            DownloadThread* thread_ptr;
            ~ScopeGuard() {
                thread_ptr->m_row.on_download_finished();
            }
        } guard{this};

        try {
            // std::this_thread::sleep_for(std::chrono::seconds(7));
            RemoteMetadata& remote_handle = m_row.fetch_remote_metadata();
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

            m_row.update_table_label(true);

        }catch(OperationCancelledException&){
            // user cancelled action

            m_row.update_table_label(false);

        }catch(OperationFailedException&){
            m_row.update_table_label(false);

            m_row.report_download_failed();
        }catch(...){
            m_row.update_table_label(false);

            m_row.report_exception_caught("ResourceDownloadButton::start_download");
        }

    }
    );

}

void DownloadThread::run_download(DownloadedResourceMetadata resource_metadata){
    Logger& logger = global_logger_tagged();
    // std::this_thread::sleep_for(std::chrono::seconds(5));

    std::string url = resource_metadata.url;
    std::string resource_name = resource_metadata.resource_name;
    qint64 expected_size = resource_metadata.size_compressed_bytes;

    std::string resource_directory = DOWNLOADED_RESOURCE_PATH() + resource_name;
    try{

        // delete directory and the old resource
        fs::remove_all(Filesystem::Path(resource_directory));

        // download
        std::string zip_path = resource_directory + "/temp.zip";
        FileDownloader::download_file_to_disk(
            *this,
            logger, 
            url, 
            zip_path,
            expected_size,
            [this](int percentage_progress){
                m_row.report_download_progress(percentage_progress);
            }
        );

        // hash
        std::string hash = 
            hash_file(
                *this,
                zip_path,
                [this](int percentage_progress){
                    m_row.report_hash_progress(percentage_progress);
                }
            );
        std::string expected_hash = resource_metadata.sha_256;
        if (hash != expected_hash){
            std::cerr << "current hash: " << hash << endl;
            throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
                "Downloaded file failed verification. SHA 256 hash did not match the expected value.");
        }

        // Filesystem::Path p{zip_path};
        // cout << "File size: " << std::filesystem::file_size(p) << endl;

        // unzip
        unzip_file(
            *this,
            zip_path.c_str(), 
            resource_directory.c_str(),
            [this](int percentage_progress){
                m_row.report_unzip_progress(percentage_progress);
            }
        );

        // delete old zip file
        fs::remove(Filesystem::Path(zip_path));

        throw_if_cancelled();

    }catch(OperationCancelledException&){
        // delete directory and the resource
        fs::remove_all(Filesystem::Path(resource_directory));

        throw;
    }catch(OperationFailedException&){
        // delete directory and the resource
        fs::remove_all(Filesystem::Path(resource_directory));

        throw;
    }catch(...){
        // delete directory and the resource
        fs::remove_all(Filesystem::Path(resource_directory));

        throw;
    }

}

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
        , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, tostr_bytes(file_size))
        , m_is_downloaded(is_downloaded)
        , m_is_downloaded_label(LockMode::LOCK_WHILE_RUNNING, is_downloaded_string(is_downloaded))
        , m_version_num(version_num)
        , m_version_status(version_status)
        , m_version_status_label(LockMode::LOCK_WHILE_RUNNING, resource_version_to_string(version_status))
    {}

    ListenerSet<DownloadListener> listeners;

    LabelCellOption m_resource_name;

    size_t m_file_size;
    LabelCellOption m_file_size_label;

    bool m_is_downloaded;
    LabelCellOption m_is_downloaded_label;

    std::optional<uint16_t> m_version_num;
    ResourceVersionStatus m_version_status;
    LabelCellOption m_version_status_label;


};

void ResourceDownloadRow::set_version_status(ResourceVersionStatus version_status){
    m_data->m_version_status = version_status;
    m_data->m_version_status_label.set_text(resource_version_to_string(version_status));
}


void ResourceDownloadRow::set_is_downloaded(bool is_downloaded){
    m_data->m_is_downloaded = is_downloaded;
    m_data->m_is_downloaded_label.set_text(is_downloaded_string(is_downloaded));
}

void ResourceDownloadRow::update_table_label(bool success){
    set_is_downloaded(success);
    set_version_status(success ? ResourceVersionStatus::CURRENT : ResourceVersionStatus::NOT_APPLICABLE);
}



ResourceDownloadRow::~ResourceDownloadRow(){
    // cout << "~ResourceDownloadRow" << endl;
    m_worker1.wait_and_ignore_exceptions();
    m_worker2.wait_and_ignore_exceptions();
}
ResourceDownloadRow::ResourceDownloadRow(
    DownloadedResourceMetadata local_metadata,
    bool is_downloaded,
    std::optional<uint16_t> version_num,
    ResourceVersionStatus version_status
)
    : StaticTableRow(local_metadata.resource_name)
    , m_button_state(ButtonState::READY)
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

            // update_button_state(ButtonState::READY);
            report_metadata_fetch_finished(predownload_warning);

        }catch(OperationFailedException&){
            // cout << "failed" << endl;
            // update_table_label(false);
            update_button_state(ButtonState::READY);
            report_download_failed();
            return;
        }catch(...){
            // update_table_label(false);
            update_button_state(ButtonState::READY);
            // cout << "Exception thrown in thread" << endl;
            report_exception_caught("ResourceDownloadButton::ensure_remote_metadata_loaded");
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

        std::string disk_space_requirement = "This will require " + tostr_bytes(decompressed_size + compressed_size) + " of free space";

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

    if (m_download_thread == nullptr){
        m_download_thread = DownloadThread::create(*this);  //std::make_shared<DownloadThread>(*this);
        m_download_thread->start_download_thread();
    }
}


void ResourceDownloadRow::start_delete(){
    m_worker2 = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            std::string resource_name = m_local_metadata.resource_name;

            std::string resource_directory = DOWNLOADED_RESOURCE_PATH() + resource_name;
            // delete directory and the old resource
            fs::remove_all(Filesystem::Path(resource_directory));

            // update the table labels
            set_is_downloaded(false);
            set_version_status(ResourceVersionStatus::NOT_APPLICABLE);
            
            update_button_state(ButtonState::READY);
        }catch(...){
            update_button_state(ButtonState::READY);
            report_exception_caught("ResourceDownloadButton::start_delete");
            return;
        }
    }
    );

}

void ResourceDownloadRow::on_download_finished(){
    
    update_button_state(ButtonState::READY);
    {
        std::lock_guard<std::mutex> lock(m_thread_mutex);
        if (m_download_thread){
            // cout << "reset m_download_thread" << endl;

            // This releases the Row's ownership. 
            // The object will actually delete itself once the lambda 
            // in start_download_thread() finishes (releasing 'self').
            m_download_thread.reset();
        }
    }
}

void ResourceDownloadRow::update_button_state(ButtonState state){
    switch (state){
    case ButtonState::DOWNLOAD:
        // button state can only enter the DOWNLOAD state 
        // if going from the READY state
        if (m_button_state == ButtonState::READY){
            m_download_button.set_enabled(false);
            m_delete_button.set_enabled(false);
            m_cancel_button.set_enabled(true);
            m_button_state = state;
        }
        break;
    case ButtonState::DELETE:
        // button state can only enter the DELETE state 
        // if going from the READY state
        if (m_button_state == ButtonState::READY){
            m_download_button.set_enabled(false);
            m_delete_button.set_enabled(false);
            m_cancel_button.set_enabled(false);
            m_button_state = state;
        }
        break;
    case ButtonState::CANCEL:
        // button state can only enter the CANCEL state 
        // if going from the DOWNLOAD state
        if (m_button_state == ButtonState::DOWNLOAD){ 
            m_download_button.set_enabled(false);
            m_delete_button.set_enabled(false);
            m_cancel_button.set_enabled(false);
            if (m_download_thread){
                m_download_thread->cancel();  // cancel the download thread
            }
            m_button_state = state;
        }
        break;
    case ButtonState::READY:
        m_download_button.set_enabled(true);
        m_delete_button.set_enabled(true);
        m_cancel_button.set_enabled(true);
        m_button_state = state;
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "update_button_state: Unknown enum.");  
    }

    report_button_state_updated();
}

void ResourceDownloadRow::add_listener(DownloadListener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.add(listener);
}
void ResourceDownloadRow::remove_listener(DownloadListener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.remove(listener);
}

void ResourceDownloadRow::report_download_progress(int percentage){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_download_progress, percentage);
}
void ResourceDownloadRow::report_unzip_progress(int percentage){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_unzip_progress, percentage);
}
void ResourceDownloadRow::report_hash_progress(int percentage){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_hash_progress, percentage);
}

void ResourceDownloadRow::report_metadata_fetch_finished(const std::string& popup_message){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_metadata_fetch_finished, popup_message);
}
void ResourceDownloadRow::report_exception_caught(const std::string& function_name){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_exception_caught, function_name);
}
void ResourceDownloadRow::report_download_failed(){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_download_failed);
}

void ResourceDownloadRow::report_button_state_updated(){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_button_state_updated);
}


}
