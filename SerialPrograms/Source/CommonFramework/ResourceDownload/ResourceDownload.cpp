/*  Required Download
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/GlobalSettingsPanel.h"
#include "GlobalResourceDownloadManager.h"
#include "ResourceDownload.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

ResourceDownload::~ResourceDownload(){
    m_download_thread.cancel();
}

ResourceDownload::ResourceDownload(GlobalResourceDownloadManager& download_manager, DownloadedResourceMetadata resource_metadata, Mutex& lock, ConditionVariable& cv)
    : m_resource_metadata(resource_metadata)
    , m_name(resource_metadata.resource_name)
    , m_lock(lock)
    , m_cv(cv)
    , m_download_thread(initialize_download_thread())
{
    // start_download();
}

DownloadThread ResourceDownload::initialize_download_thread(){

    DownloadThread::Hooks generic_row_hooks{
        .is_ready_to_start = [this] { return GlobalResourceDownloadManager::instance().is_download_ready_to_start(m_name); },
        .on_finished       = [this](bool success) { 
            report_download_finished(success); 
        },
        .report_failed     = [this] { report_download_failed(); },
        .report_exception_caught  = [this](const char* error_msg) { report_unexpected_exception_caught(error_msg); },
        .report_download_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_download_progress(bytes_done, total_bytes); },
        .report_hash_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_hash_progress(bytes_done, total_bytes); },
        .report_unzip_progress  = [this](uint64_t bytes_done, uint64_t total_bytes) { report_unzip_progress(bytes_done, total_bytes); }
    };

    return DownloadThread{generic_row_hooks, m_resource_metadata, m_lock, m_cv};

}



void ResourceDownload::start_download(){
    cout << "start_download" << endl;
    m_download_thread.start_download_thread();
}

void ResourceDownload::cancel_download(){
    m_download_thread.cancel();
    // m_parent_session.remove_from_download_list(m_index);
}



//////////////////////////
// Listeners
//////////////////////////
void ResourceDownload::add_listener(Listener& listener){
    m_listeners.add(listener);
}
void ResourceDownload::remove_listener(Listener& listener){
    m_listeners.remove(listener);
} 

// void ResourceDownload::report_download_started(){
//     m_listeners.run_method(&Listener::on_download_started);
// }

void ResourceDownload::report_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    m_listeners.run_method(&Listener::on_download_progress, bytes_done, total_bytes);
}
void ResourceDownload::report_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    m_listeners.run_method(&Listener::on_unzip_progress, bytes_done, total_bytes);
}
void ResourceDownload::report_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    m_listeners.run_method(&Listener::on_hash_progress, bytes_done, total_bytes);
}

void ResourceDownload::report_download_finished(bool success){
    m_listeners.run_method(&Listener::on_download_finished, success, m_name);

}

void ResourceDownload::report_download_failed(){
    m_listeners.run_method(&Listener::on_download_failed, m_name);
}

void ResourceDownload::report_unexpected_exception_caught(const std::string& error_msg){
    m_listeners.run_method(&Listener::on_exception_caught, error_msg);
}

}
