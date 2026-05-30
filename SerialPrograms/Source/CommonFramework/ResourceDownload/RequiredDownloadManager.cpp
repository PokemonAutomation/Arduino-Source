/*  Required Download Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ResourceDownload/ResourceDownloadHelpers.h"
#include "CommonFramework/ResourceDownload/RequiredDownload.h"
#include "RequiredDownloadManager.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


RequiredDownloadManager::RequiredDownloadManager(const std::vector<std::string>& required_resources)
    : m_required_resources(required_resources)
{}

void RequiredDownloadManager::initialize_required_downloads(){
    // const std::vector<std::string>& required_resources = {"PokemonSV/AreaZero", "PaddleOCR" };
    auto [downloads, upgrade_needed] = find_resources_to_download(m_required_resources);

    // If an exception happened above, state variables below are never touched

    std::vector<std::string> download_queue;
    // Prevent multiple reallocations
    download_queue.reserve(downloads.size());  
    for (auto download : downloads){ // (re-)initialize m_download_queue
        download_queue.push_back(download->get_name());
    }
    m_download_queue = std::move(download_queue);
    m_required_downloads = std::move(downloads);
    m_upgrade_warning = upgrade_needed;
    m_resource_list_initialized = true;
}

RequiredResourceResult RequiredDownloadManager::find_resources_to_download(const std::vector<std::string>& required_resources){
    std::vector<DownloadedResourceMetadata> local_resources = local_resource_download_list();
    std::vector<DownloadedResourceMetadata> remote_resources = remote_resource_download_list();

    bool upgrade_warning = false;

    // find out version status for each required resource type, if outdated, or not downloaded,
    // check the version available for download in the remote list and add the corresponding 
    // remote_resource to the required_download_list
    std::vector<std::shared_ptr<RequiredDownload>> required_download_list;
    for(const std::string& resource_type : required_resources){
        DownloadedResourceMetadata expected_resource = get_resource_metadata_from_resource_type(resource_type, local_resources);
        ResourceVersionStatus version_status = get_version_status(expected_resource);

        switch(version_status){
        case ResourceVersionStatus::CURRENT:
            // we have this resource, check the next one
            continue;
        case ResourceVersionStatus::FUTURE_VERSION:
            // we have this resource, check the next one
            // however, the resource that was downloaded is more updated than what the program is expecting
            // warn the user to upgrade CC.
            upgrade_warning = true;
            continue;
        case ResourceVersionStatus::OUTDATED:
        case ResourceVersionStatus::NOT_APPLICABLE:{
            // we don't have the resource. check remote to see if correct version is available.
            DownloadedResourceMetadata remote_resource = get_resource_metadata_from_resource_type(resource_type, remote_resources);
            uint16_t expected_version_num = expected_resource.version_num.value();
            uint16_t remote_version_num = remote_resource.version_num.value();

            if (expected_version_num == remote_version_num){
                // remote version matches what we expect
                // add the resource to the download list
                // m_required_downloads.emplace_back(RequiredDownload{remote_resource});
                required_download_list.emplace_back(std::make_shared<RequiredDownload>(*this, remote_resource, m_queue_lock, m_cv));

            }else if (expected_version_num < remote_version_num){
                // remote version is more updated than we expect
                // warn the user to upgrade CC.
                // regardless, add the resource to the download list
                // m_required_downloads.emplace_back(RequiredDownload{remote_resource});
                required_download_list.emplace_back(std::make_shared<RequiredDownload>(*this, remote_resource, m_queue_lock, m_cv));
                upgrade_warning = true;

            }else if (expected_version_num > remote_version_num){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "resources_to_download: expected_version_num > remote_version_num. This shouldn't happen."); 
            }
            break;
        }
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "resources_to_download: Unknown enum."); 
        }
    }

    // m_upgrade_warning = upgrade_warning;
    // m_resource_list_initialized = true;

    return RequiredResourceResult{std::move(required_download_list), upgrade_warning};

}

bool RequiredDownloadManager::get_upgrade_warning(){
    if (!m_resource_list_initialized){
        initialize_required_downloads();
    }

    return m_upgrade_warning;
}

const std::vector<std::shared_ptr<RequiredDownload>>& RequiredDownloadManager::get_required_downloads(){
    if (!m_resource_list_initialized){
        initialize_required_downloads();
    }

    return m_required_downloads;
}


void RequiredDownloadManager::remove_from_download_list(const std::string& resource_slug){
    std::lock_guard<Mutex> lg(m_queue_lock);

    // this requires C++20
    std::erase(m_download_queue, resource_slug);
    m_cv.notify_all();
}

bool RequiredDownloadManager::is_download_ready_to_start(const std::string& resource_slug){
    // ASSUMES: the calling thread holds the m_lock. therefore, this function doesn't lock the mutex when accessing download_queue.
    // std::lock_guard<Mutex> lg(m_lock); 

    uint16_t MAX_CONCURRENT_DOWNLOADS = 10;
    return is_resource_ready_in_queue(MAX_CONCURRENT_DOWNLOADS, resource_slug, m_download_queue);
}

void RequiredDownloadManager::check_if_all_downloads_done(){
    std::lock_guard<Mutex> lg(m_queue_lock);

    // we don't add to the download queue, 
    // so when the queue is empty, we must be done
    if (m_download_queue.empty()){
        report_all_downloads_finished();
    }
}

void RequiredDownloadManager::cancel_downloads(){
    for (auto download : m_required_downloads){
        download->cancel_download();
    }
}

//////////////////////////
// Listeners
//////////////////////////
void RequiredDownloadManager::add_download_listener(DownloadListener& listener){
    m_download_listeners.add(listener);
}
void RequiredDownloadManager::remove_download_listener(DownloadListener& listener){
    m_download_listeners.remove(listener);
}

void RequiredDownloadManager::report_all_downloads_finished(){
    m_download_listeners.run_method(&DownloadListener::on_all_downloads_finished);
}
void RequiredDownloadManager::report_download_failed(){
    m_download_listeners.run_method(&DownloadListener::on_download_failed);
}
void RequiredDownloadManager::report_unexpected_exception_caught(const std::string& function_name){
    m_download_listeners.run_method(&DownloadListener::on_exception_caught, function_name);
}

}
