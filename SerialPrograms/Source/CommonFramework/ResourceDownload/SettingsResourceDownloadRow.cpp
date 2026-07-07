/*  Resource Download Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Filesystem.h"
#include "Common/Cpp/ScopeExit.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ResourceDownload/GlobalResourceDownloadManager.h"
#include "ResourceDownloadHelpers.h"
// #include "SettingsResourceDownloadTable.h"
#include "SettingsResourceDownloadRow.h"

// #include <thread>
// #include <fstream>
#include <filesystem>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

    namespace fs = std::filesystem;



/////////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsResourceDownloadRow
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

struct SettingsResourceDownloadRow::Data{
    Data(
        DownloadedResourceMetadata expected_metadata,
        bool is_downloaded,
        std::optional<uint16_t> version_num,
        ResourceVersionStatus version_status
    )
        : m_local_metadata(expected_metadata)
        , m_action_state(ActionState::READY)
        , m_resource_slug(expected_metadata.resource_name)
        , m_resource_name_label(LockMode::LOCK_WHILE_RUNNING, m_resource_slug)
        , m_file_size(expected_metadata.size_decompressed_bytes)
        , m_file_size_label(LockMode::LOCK_WHILE_RUNNING, tostr_bytes(m_file_size))
        , m_is_downloaded(is_downloaded)
        , m_is_downloaded_label(LockMode::LOCK_WHILE_RUNNING, is_downloaded_string(is_downloaded))
        , m_version_num(version_num)
        , m_version_status(version_status)
        , m_version_status_label(LockMode::LOCK_WHILE_RUNNING, resource_version_to_string(version_status))
    {}



    DownloadedResourceMetadata m_local_metadata;

    ActionState m_action_state;

    std::string m_resource_slug;
    LabelCellOption m_resource_name_label;

    size_t m_file_size;
    LabelCellOption m_file_size_label;

    bool m_is_downloaded;
    LabelCellOption m_is_downloaded_label;

    std::optional<uint16_t> m_version_num;
    ResourceVersionStatus m_version_status;
    LabelCellOption m_version_status_label;

    AsyncTask m_pre_download_thread;
    AsyncTask m_delete_thread;

    std::shared_ptr<ResourceDownload> m_download_ptr;

    std::optional<DownloadedResourceMetadata> m_cached_metadata;

    Mutex m_action_state_lock;


    ListenerSet<Listener> listeners;



};

void SettingsResourceDownloadRow::set_version_status(ResourceVersionStatus version_status){
    m_data->m_version_status = version_status;
    m_data->m_version_status_label.set_text(resource_version_to_string(version_status));
}

std::string SettingsResourceDownloadRow::get_resource_slug(){ 
    return m_data->m_resource_slug; 
}

void SettingsResourceDownloadRow::set_is_downloaded(bool is_downloaded){
    m_data->m_is_downloaded = is_downloaded;
    m_data->m_is_downloaded_label.set_text(is_downloaded_string(is_downloaded));
}

void SettingsResourceDownloadRow::update_table_label(bool success){
    set_is_downloaded(success);
    set_version_status(success ? ResourceVersionStatus::CURRENT : ResourceVersionStatus::NOT_APPLICABLE);
}



SettingsResourceDownloadRow::~SettingsResourceDownloadRow(){
    // cout << "~SettingsResourceDownloadRow" << endl;
    if (m_data->m_download_ptr) {
        m_data->m_download_ptr->remove_listener(*this); 
    }
    m_data->m_pre_download_thread.wait_and_ignore_exceptions();
    m_data->m_delete_thread.wait_and_ignore_exceptions();
}
SettingsResourceDownloadRow::SettingsResourceDownloadRow(
    DownloadedResourceMetadata local_metadata,
    bool is_downloaded,
    std::optional<uint16_t> version_num,
    ResourceVersionStatus version_status
)
    : StaticTableRow(local_metadata.resource_name)
    , m_data(CONSTRUCT_TOKEN, local_metadata, is_downloaded, version_num, version_status)
    , m_download_button(*this)
    , m_delete_button(*this)
    , m_cancel_button(*this)
    , m_progress_bar(*this)
    , m_popup(*this)
{
    PA_ADD_STATIC(m_data->m_resource_name_label);
    PA_ADD_STATIC(m_data->m_file_size_label);
    PA_ADD_STATIC(m_data->m_is_downloaded_label);
    PA_ADD_STATIC(m_data->m_version_status_label);

    PA_ADD_STATIC(m_download_button);
    PA_ADD_STATIC(m_delete_button);
    PA_ADD_STATIC(m_cancel_button);
    PA_ADD_STATIC(m_progress_bar);
    PA_ADD_STATIC(m_popup);

    update_action_state(ActionState::READY);
}



const DownloadedResourceMetadata& SettingsResourceDownloadRow::fetch_remote_metadata(){

    if (m_data->m_cached_metadata.has_value()){
        return m_data->m_cached_metadata.value();
    }

    Logger& logger = global_logger_tagged();
    std::vector<DownloadedResourceMetadata> all_remote_metadata;

    try{
        all_remote_metadata = remote_resource_download_list();
    }catch(OperationFailedException&){
        std::cerr << "SettingsResourceDownloadRow::fetch_remote_metadata: Error" << endl;
        throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
            "Error: Download failed. Failed to fetch the list of available downloads. Check your internet connection.");
    }

    std::string resource_name = m_data->m_resource_name_label.text();

    for (const DownloadedResourceMetadata& remote_metadata : all_remote_metadata){
        if (remote_metadata.resource_name == resource_name){
            m_data->m_cached_metadata = remote_metadata;
            return m_data->m_cached_metadata.value();
        }
    }

    // if corresponding remote_metadata not found
    throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, 
            "fetch_remote_metadata: Resource no longer available for download. We recommend updating the Computer Control program.");

}



void SettingsResourceDownloadRow::ensure_remote_metadata_loaded(){
    m_data->m_pre_download_thread = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            if (!is_given_action_state(ActionState::PRE_DOWNLOAD)){
                return;
            }


            bool success = false;
            ScopeExit on_exit([&success, this]{
                if (!success){
                    update_action_state(ActionState::READY);
                }
            });
            
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            std::string predownload_warning;
            try {
                const DownloadedResourceMetadata& remote_metadata = fetch_remote_metadata();
                predownload_warning = predownload_warning_summary(remote_metadata);
            }catch(OperationFailedException& e){
                predownload_warning = e.message();
            }
            // cout << "Fetched remote metadata" << endl;
            // throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "testing");
            // Logger& logger = global_logger_tagged();
            // throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, "test");

            report_metadata_fetch_finished(predownload_warning);
            success = true;

        }catch(OperationFailedException&){
            // cout << "failed" << endl;
            GlobalResourceDownloadManager::instance().report_download_failed(m_data->m_resource_slug);
            return;
        }catch(...){
            // cout << "Exception thrown in thread" << endl;
            GlobalResourceDownloadManager::instance().report_unexpected_exception_caught("Error: SettingsResourceDownloadButton::ensure_remote_metadata_loaded: Unknown exception. Report this as an error.");
            return;
        }
    
    }
    );

}

std::string SettingsResourceDownloadRow::predownload_warning_summary(const DownloadedResourceMetadata& remote_metadata){

    std::string predownload_warning;

    uint16_t local_version_num = m_data->m_local_metadata.version_num.value();

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



void SettingsResourceDownloadRow::start_download(){
    if (!is_given_action_state(ActionState::PRE_DOWNLOAD)){
        return;
    }

    update_action_state(ActionState::DOWNLOADING);

    cancel_download_thread(); // cancels old download thread

    try{
        std::shared_ptr<ResourceDownload> download_ptr = GlobalResourceDownloadManager::instance().add_to_download_list(m_data->m_resource_slug);

    }catch(OperationFailedException&){
        update_action_state(ActionState::READY);
    }
    
}


void SettingsResourceDownloadRow::start_delete(){
    m_data->m_delete_thread = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
    [this]{ 
        try {
            ScopeExit on_exit([this]{
                update_table_label(false);
                update_action_state(ActionState::READY);
            });

            // throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "test.");
            // Logger& logger = global_logger_tagged();
            // throw_and_log<OperationFailedException>(logger, ErrorReport::NO_ERROR_REPORT, "test");

            if (!is_given_action_state(ActionState::PRE_DELETE)){
                return;
            }
            update_action_state(ActionState::DELETING);

            std::string resource_name = m_data->m_local_metadata.resource_name;

            std::string resource_directory = DOWNLOADED_RESOURCE_PATH() + resource_name;
            // delete directory and the old resource
            fs::remove_all(Filesystem::Path(resource_directory));

        }catch(OperationFailedException& e){
            std::cerr << e.message() << endl;
            GlobalResourceDownloadManager::instance().report_unexpected_exception_caught(
                "Error: SettingsResourceDownloadButton::start_delete: Unexpected OperationFailedException exception. Report this as an error.");
            return;
        }catch(...){
            GlobalResourceDownloadManager::instance().report_unexpected_exception_caught(
                "Error: SettingsResourceDownloadButton::start_delete: Unknown exception. Report this as an error.");
            return;
        }
    }
    );

}


void SettingsResourceDownloadRow::update_action_state(ActionState state){
    std::lock_guard<Mutex> lock(m_data->m_action_state_lock);
    {
        switch (state){
        case ActionState::PRE_DOWNLOAD:
            // action state can only enter the PRE_DOWNLOAD state 
            // if going from the READY state
            if (m_data->m_action_state == ActionState::READY){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(true);
                m_progress_bar.change_text("Downloading");
                m_progress_bar.set_visibility(ConfigOptionState::ENABLED);
                m_data->m_action_state = state;
                cout << "ActionState::PRE_DOWNLOAD" << endl;
            }
            break;
        case ActionState::DOWNLOADING:
            if (m_data->m_action_state == ActionState::PRE_DOWNLOAD || m_data->m_action_state == ActionState::PRE_CANCEL || m_data->m_action_state == ActionState::READY){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(true);
                m_progress_bar.change_text("Downloading");
                m_progress_bar.set_visibility(ConfigOptionState::ENABLED);
                m_data->m_action_state = state;
                cout << "ActionState::DOWNLOADING" << endl;
            }
            break;
        case ActionState::PRE_DELETE:
            // action state can only enter the PRE_DELETE state 
            // if going from the READY state
            if (m_data->m_action_state == ActionState::READY){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_progress_bar.reset_progress();
                m_data->m_action_state = state;
                cout << "ActionState::PRE_DELETE" << endl;
            }
            break;
        case ActionState::DELETING:
            // action state can only enter the DELETING state 
            // if going from the PRE_DELETE state
            if (m_data->m_action_state == ActionState::PRE_DELETE){
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_progress_bar.reset_progress();
                m_data->m_action_state = state;
                cout << "ActionState::DELETING" << endl;
            }
            break;
        case ActionState::PRE_CANCEL:
            // action state can only enter the PRE_CANCEL state 
            // if going from the DOWNLOADING state
            if (m_data->m_action_state == ActionState::DOWNLOADING){ 
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_progress_bar.reset_progress();
                m_data->m_action_state = state;
                cout << "ActionState::PRE_CANCEL" << endl;
            }
            break;
        case ActionState::CANCELLING:
            // action state can only enter the CANCELLING state 
            // if going from the PRE_CANCEL state
            if (m_data->m_action_state == ActionState::PRE_CANCEL){ 
                m_download_button.set_enabled(false);
                m_delete_button.set_enabled(false);
                m_cancel_button.set_enabled(false);
                m_progress_bar.reset_progress();
                m_data->m_action_state = state;
                cout << "ActionState::CANCELLING" << endl;
            }
            break;
        case ActionState::READY:
            m_download_button.set_enabled(true);
            m_delete_button.set_enabled(true);
            m_cancel_button.set_enabled(true);
            m_progress_bar.change_text("");
            m_progress_bar.set_visibility(ConfigOptionState::HIDDEN);
            m_progress_bar.reset_progress();
            m_data->m_action_state = state;
            cout << "ActionState::READY" << endl;
            break;
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "update_action_state: Unknown enum.");  
        }
    }

    report_action_state_updated();
}

ActionState SettingsResourceDownloadRow::get_action_state(){
    std::lock_guard<Mutex> lock(m_data->m_action_state_lock);
    return m_data->m_action_state;
}

bool SettingsResourceDownloadRow::is_given_action_state(ActionState state){
    std::lock_guard<Mutex> lock(m_data->m_action_state_lock);
    return m_data->m_action_state == state;
}

void SettingsResourceDownloadRow::cancel_download_thread(){
    if (m_data->m_download_ptr){ // if download is active
        m_data->m_download_ptr->cancel_download();
    }
}


void SettingsResourceDownloadRow::connect_with_download(std::shared_ptr<ResourceDownload> download_ptr){
    if (m_data->m_download_ptr){
        m_data->m_download_ptr->remove_listener(*this);
    }
    m_data->m_download_ptr = std::move(download_ptr);
    m_data->m_download_ptr->add_listener(*this);
    m_data->m_download_ptr->add_listener(m_progress_bar);
    update_action_state(ActionState::DOWNLOADING);
}


/////////////////////////
// Listener
/////////////////////////
void SettingsResourceDownloadRow::add_listener(Listener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.add(listener);
}
void SettingsResourceDownloadRow::remove_listener(Listener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.remove(listener);
}

void SettingsResourceDownloadRow::report_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&Listener::on_download_progress, bytes_done, total_bytes);
}
void SettingsResourceDownloadRow::report_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&Listener::on_unzip_progress, bytes_done, total_bytes);
}
void SettingsResourceDownloadRow::report_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&Listener::on_hash_progress, bytes_done, total_bytes);
}

void SettingsResourceDownloadRow::report_metadata_fetch_finished(const std::string& popup_message){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&Listener::on_metadata_fetch_finished, popup_message);
}

void SettingsResourceDownloadRow::report_action_state_updated(){
    auto scope = m_lifetime_sanitizer.check_scope();
    m_data->listeners.run_method(&Listener::on_action_state_updated);
}


////////////////////////////////
// ResourceDownload::Listener
////////////////////////////////
void SettingsResourceDownloadRow::on_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    report_download_progress(bytes_done, total_bytes);
}
void SettingsResourceDownloadRow::on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    report_unzip_progress(bytes_done, total_bytes);
}
void SettingsResourceDownloadRow::on_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    report_hash_progress(bytes_done, total_bytes);
}

void SettingsResourceDownloadRow::on_download_finished(bool success, const std::string& resource_slug){
    // we can't run `download_ptr->remove_listener(*this)` in this function
    // since it results in deadlock, since this function is part of the listener loop
    m_data->m_download_ptr.reset();

    update_action_state(ActionState::READY);
    update_table_label(success);
}


}
