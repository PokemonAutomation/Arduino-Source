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
    m_pre_download_thread.wait_and_ignore_exceptions();
    m_delete_thread.wait_and_ignore_exceptions();
}
ResourceDownloadRow::ResourceDownloadRow(
    ResourceDownloadTable& parent_table,
    Mutex& lock,
    ConditionVariable& cv,
    std::string resource_slug,
    DownloadedResourceMetadata local_metadata,
    bool is_downloaded,
    std::optional<uint16_t> version_num,
    ResourceVersionStatus version_status
)
    : StaticTableRow(local_metadata.resource_name)
    , m_parent_table(parent_table)
    , m_download_lock(lock)
    , m_download_cv(cv)
    , m_action_state(ActionState::READY)
    , m_resource_slug(resource_slug)
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



const DownloadedResourceMetadata& ResourceDownloadRow::fetch_remote_metadata(){

    if (m_cached_metadata.has_value()){
        return m_cached_metadata.value();
    }

    std::vector<DownloadedResourceMetadata> all_remote_metadata = remote_resource_download_list();

    std::string resource_name = m_data->m_resource_name.text();

    for (const DownloadedResourceMetadata& remote_metadata : all_remote_metadata){
        if (remote_metadata.resource_name == resource_name){
            m_cached_metadata = remote_metadata;
            return m_cached_metadata.value();
        }
    }

    // if corresponding remote_metadata not found
    Logger& logger = global_logger_tagged();
    throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
            "fetch_remote_metadata: Resource no longer available for download. We recommend updating the Computer Control program..");

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
    m_pre_download_thread = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            if (!is_given_action_state(ActionState::PRE_DOWNLOAD)){
                return;
            }
            
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            std::string predownload_warning;
            try {
                const DownloadedResourceMetadata& remote_metadata = fetch_remote_metadata();
                predownload_warning = predownload_warning_summary(remote_metadata);
            }catch(OperationFailedException&){
                predownload_warning = "Resource no longer available for download. We recommend updating the Computer Control program.";
            }
            // cout << "Fetched remote metadata" << endl;
            // throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "testing"); 

            // update_action_state(ActionState::READY);
            report_metadata_fetch_finished(predownload_warning);

        }catch(OperationFailedException&){
            // cout << "failed" << endl;
            // update_table_label(false);
            update_action_state(ActionState::READY);
            report_download_failed();
            return;
        }catch(...){
            // update_table_label(false);
            update_action_state(ActionState::READY);
            // cout << "Exception thrown in thread" << endl;
            report_exception_caught("ResourceDownloadButton::ensure_remote_metadata_loaded");
            return;
        }
    
    }
    );

}

std::string ResourceDownloadRow::predownload_warning_summary(const DownloadedResourceMetadata& remote_metadata){

    std::string predownload_warning;

    uint16_t local_version_num = m_local_metadata.version_num.value();

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

    return predownload_warning;
}



void ResourceDownloadRow::start_download(){
    if (!is_given_action_state(ActionState::PRE_DOWNLOAD)){
        return;
    }

    update_action_state(ActionState::DOWNLOADING);

    cancel_download_thread(); // cancels old download thread

    try{
        const DownloadedResourceMetadata& remote_metadata = fetch_remote_metadata();
        DownloadThread::Hooks generic_row_hooks{
            .is_ready_to_start = [this] { return is_download_ready_to_start(); },
            .on_finished       = [this](bool success) { 
                update_table_label(success);
                on_download_finished(); 
            },
            // .run_download      = [this](auto& meta) { this->run_download(meta); },
            // .update_label      = [this](bool success) { row.update_table_label(success); },
            .report_failed     = [this] { report_download_failed(); },
            .report_exception_caught  = [this](const char* context) { report_exception_caught(context); },
            .report_download_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_download_progress(bytes_done, total_bytes); },
            .report_hash_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_hash_progress(bytes_done, total_bytes); },
            .report_unzip_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_unzip_progress(bytes_done, total_bytes); }
        };

        m_parent_table.add_row_to_download_list(m_resource_slug);
        m_download_thread = std::make_unique<DownloadThread>(generic_row_hooks, remote_metadata, m_download_lock, m_download_cv);
        m_download_thread->start_download_thread();
    }catch(OperationFailedException&){
        update_action_state(ActionState::READY);
    }
    
}


void ResourceDownloadRow::start_delete(){
    m_delete_thread = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            if (!is_given_action_state(ActionState::PRE_DELETE)){
                return;
            }
            update_action_state(ActionState::DELETING);

            std::string resource_name = m_local_metadata.resource_name;

            std::string resource_directory = DOWNLOADED_RESOURCE_PATH() + resource_name;
            // delete directory and the old resource
            fs::remove_all(Filesystem::Path(resource_directory));

            // update the table labels
            set_is_downloaded(false);
            set_version_status(ResourceVersionStatus::NOT_APPLICABLE);
            
            update_action_state(ActionState::READY);
        }catch(...){
            update_action_state(ActionState::READY);
            report_exception_caught("ResourceDownloadButton::start_delete");
            return;
        }
    }
    );

}

void ResourceDownloadRow::on_download_finished(){
    
    update_action_state(ActionState::READY);
    remove_self_from_download_queue();
}

void ResourceDownloadRow::update_action_state(ActionState state){
    std::lock_guard<Mutex> lock(m_action_state_lock);
    {
        switch (state){
        case ActionState::PRE_DOWNLOAD:
            // action state can only enter the PRE_DOWNLOAD state 
            // if going from the READY state
            if (m_action_state == ActionState::READY){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(true);
                m_action_state = state;
                cout << "ActionState::PRE_DOWNLOAD" << endl;
            }
            break;
        case ActionState::DOWNLOADING:
            if (m_action_state == ActionState::PRE_DOWNLOAD || m_action_state == ActionState::PRE_CANCEL){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(true);
                m_action_state = state;
                cout << "ActionState::DOWNLOADING" << endl;
            }
            break;
        case ActionState::PRE_DELETE:
            // action state can only enter the PRE_DELETE state 
            // if going from the READY state
            if (m_action_state == ActionState::READY){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_action_state = state;
                cout << "ActionState::PRE_DELETE" << endl;
            }
            break;
        case ActionState::DELETING:
            // action state can only enter the DELETING state 
            // if going from the PRE_DELETE state
            if (m_action_state == ActionState::PRE_DELETE){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_action_state = state;
                cout << "ActionState::DELETING" << endl;
            }
            break;
        case ActionState::PRE_CANCEL:
            // action state can only enter the PRE_CANCEL state 
            // if going from the DOWNLOADING state
            if (m_action_state == ActionState::DOWNLOADING){ 
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_action_state = state;
                cout << "ActionState::PRE_CANCEL" << endl;
            }
            break;
        case ActionState::CANCELLING:
            // action state can only enter the CANCELLING state 
            // if going from the PRE_CANCEL state
            if (m_action_state == ActionState::PRE_CANCEL){ 
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_action_state = state;
                cout << "ActionState::CANCELLING" << endl;
            }
            break;
        case ActionState::READY:
            m_download_button.set_enabled(true);
            m_delete_button.set_enabled(true);
            m_cancel_button.set_enabled(true);
            m_action_state = state;
            cout << "ActionState::READY" << endl;
            break;
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "update_action_state: Unknown enum.");  
        }
    }

    report_action_state_updated();
}

ActionState ResourceDownloadRow::get_action_state(){
    std::lock_guard<Mutex> lock(m_action_state_lock);
    return m_action_state;
}

void ResourceDownloadRow::add_listener(DownloadListener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.add(listener);
}
void ResourceDownloadRow::remove_listener(DownloadListener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.remove(listener);
}

void ResourceDownloadRow::report_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_download_progress, bytes_done, total_bytes);
}
void ResourceDownloadRow::report_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_unzip_progress, bytes_done, total_bytes);
}
void ResourceDownloadRow::report_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_hash_progress, bytes_done, total_bytes);
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

void ResourceDownloadRow::report_action_state_updated(){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&DownloadListener::on_action_state_updated);
}


bool ResourceDownloadRow::is_download_ready_to_start(){
    return m_parent_table.is_download_ready_to_start(m_resource_slug);
}

void ResourceDownloadRow::remove_self_from_download_queue(){
    m_parent_table.remove_row_from_download_list(m_resource_slug);
}

bool ResourceDownloadRow::is_given_action_state(ActionState state){
    std::lock_guard<Mutex> lock(m_action_state_lock);
    return m_action_state == state;
}

void ResourceDownloadRow::cancel_download_thread(){
    if (m_download_thread){ // if thread already exists
        m_download_thread->cancel(); // stop the thread
    }
}

}
