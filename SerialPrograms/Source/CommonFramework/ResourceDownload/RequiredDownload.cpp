/*  Required Download
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/GlobalSettingsPanel.h"
#include "RequiredDownloadManager.h"
#include "RequiredDownload.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


RequiredDownload::RequiredDownload(RequiredDownloadManager& download_manager, DownloadedResourceMetadata resource_metadata, Mutex& lock, ConditionVariable& cv)
    : m_download_manager(download_manager)
    , m_resource_metadata(resource_metadata)
    , m_name(resource_metadata.resource_name)
    , m_lock(lock)
    , m_cv(cv)
    , m_download_thread(initialize_download_thread())
{}

DownloadThread RequiredDownload::initialize_download_thread(){

    DownloadThread::Hooks generic_row_hooks{
        .is_ready_to_start = [this] { return m_download_manager.is_download_ready_to_start(m_name); },
        .on_finished       = [this](bool success) { 
            // GlobalSettings::instance().update_resource_download_row_status(m_index, success);
            on_download_finished(); 
        },
        .report_failed     = [this] { report_download_failed(); },
        .report_exception_caught  = [this](const char* context) { report_unexpected_exception_caught(context); },
        .report_download_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_download_progress(bytes_done, total_bytes); },
        .report_hash_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_hash_progress(bytes_done, total_bytes); },
        .report_unzip_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_unzip_progress(bytes_done, total_bytes); }
    };

    return DownloadThread{generic_row_hooks, m_resource_metadata, m_lock, m_cv};

}



void RequiredDownload::start_download(){
    m_download_thread.start_download_thread();
}

void RequiredDownload::cancel_download(){
    m_download_thread.cancel();
    // m_parent_session.remove_from_download_list(m_index);
}


void RequiredDownload::on_download_finished(){
    m_download_manager.remove_from_download_list(m_name);
    m_download_manager.check_if_all_downloads_done();
}

void RequiredDownload::report_download_failed(){
    m_download_manager.report_download_failed();
    // m_listeners.run_method(&Listener::on_download_failed);
}


void RequiredDownload::report_unexpected_exception_caught(const std::string& function_name){
    m_download_manager.report_unexpected_exception_caught(function_name);
}

//////////////////////////
// Listeners
//////////////////////////
void RequiredDownload::add_listener(Listener& listener){
    m_listeners.add(listener);
}
void RequiredDownload::remove_listener(Listener& listener){
    m_listeners.remove(listener);
} 


void RequiredDownload::report_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    m_listeners.run_method(&Listener::on_download_progress, bytes_done, total_bytes);
}
void RequiredDownload::report_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    m_listeners.run_method(&Listener::on_unzip_progress, bytes_done, total_bytes);
}
void RequiredDownload::report_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    m_listeners.run_method(&Listener::on_hash_progress, bytes_done, total_bytes);
}

}
