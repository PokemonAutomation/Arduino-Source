/*  Resource Download Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ResourceDownload/ResourceDownloadHelpers.h"
#include "CommonFramework/ResourceDownload/ResourceDownload.h"
#include "GlobalResourceDownloadManager.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


GlobalResourceDownloadManager& GlobalResourceDownloadManager::instance(){
    static GlobalResourceDownloadManager manager;
    return manager;
}


std::shared_ptr<ResourceDownload> GlobalResourceDownloadManager::add_to_download_list(const std::string& resource_slug){
    std::lock_guard<Mutex> lg(m_queue_lock);
    auto iter = std::ranges::find_if(m_download_queue, [&](const auto& download_ptr){
        return download_ptr->get_name() == resource_slug;
    });

    
    if (iter == m_download_queue.end()){ // given Resource not already within the DownloadQueue
        DownloadedResourceMetadata resource_metadata = get_remote_resource_metadata_from_resource_slug(resource_slug);
        auto download_ptr = m_download_queue.emplace_back(std::make_shared<ResourceDownload>(*this, std::move(resource_metadata), m_queue_lock, m_cv));
        
        download_ptr->add_listener(*this);
        GlobalSettings::instance().connect_row_with_download(resource_slug, download_ptr);
        download_ptr->start_download();

        return download_ptr;
    }

    // Return the existing shared_ptr found in the queue
    return *iter;


}

void GlobalResourceDownloadManager::remove_from_download_list(const std::string& resource_slug){
    // We need to delete the target ResourceDownloads outside of the lock
    // (the reason isn't clear to me. I suspect its because GlobalResourceDownloadManager is one of the
    // listeners within ResourceDownload's ListenerSet, which causes an issue during its destructor)
    // This temporary vector holds ownership of the target ResourceDownloads
    // while modifying m_download_queue occurs within the lock.
    std::vector<std::shared_ptr<ResourceDownload>> to_destroy;

    {
        std::lock_guard<Mutex> lg(m_queue_lock);

        // std::erase_if modifies the vector in-place
        std::erase_if(m_download_queue, [&](auto& download_ptr) {
            if (!download_ptr) return false;
            
            if (download_ptr->get_name() == resource_slug) {
                // Move the download_ptr out of the m_download_queue and into our temporary vector
                // This prevents immediate destruction of download_ptr, while
                // it's still removed from m_download_queue.
                to_destroy.push_back(std::move(download_ptr));
                return true;
            }
            return false;
        });
        
        m_cv.notify_all();
    }

    // The 'to_destroy' vector goes out of scope here.
    // The ResourceDownload destructors run safely out of the lock.
}

bool GlobalResourceDownloadManager::is_download_ready_to_start(const std::string& resource_slug){
    // ASSUMES: the calling thread holds the m_lock. therefore, this function doesn't lock the mutex when accessing download_queue.
    // std::lock_guard<Mutex> lg(m_lock); 

    uint16_t MAX_CONCURRENT_DOWNLOADS = 10;

    auto iter = std::ranges::find_if(m_download_queue, [&](const auto& download_ptr){
        return download_ptr->get_name() == resource_slug;
    });

    if (iter == m_download_queue.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "is_download_ready_to_start: resource_slug not found within download_queue.");
    }

    uint16_t download_position = (uint16_t)std::distance(m_download_queue.begin(), iter);

    // cout << "download_position: " << std::to_string(download_position) << endl;

    return download_position < MAX_CONCURRENT_DOWNLOADS;
}


void GlobalResourceDownloadManager::cancel_downloads(){
    std::vector<std::shared_ptr<ResourceDownload>> downloads_to_cancel;

    // Lock briefly to safely snapshot the queue
    {
        std::lock_guard<Mutex> lg(m_queue_lock);
        downloads_to_cancel = m_download_queue; 
    }

    // We must call cancel_download() outside of the lock.
    // The reason is that GlobalResourceDownloadManager is a listener to ResourceDownload. 
    // ResourceDownload::cancel_download() will trigger GlobalResourceDownloadManager::on_download_finished,
    // which then triggers GlobalResourceDownloadManager::remove_from_download_list(), which is 
    // done under a lock.
    // This prevents a deadlock.
    for (auto& download : downloads_to_cancel) {
        if (download) {
            download->cancel_download(); 
        }
    }

}


//////////////////////////
// Listener
//////////////////////////
void GlobalResourceDownloadManager::add_download_listener(Listener& listener){
    m_download_listeners.add(listener);
}
void GlobalResourceDownloadManager::remove_download_listener(Listener& listener){
    m_download_listeners.remove(listener);
}

void GlobalResourceDownloadManager::report_all_downloads_finished(){
    m_download_listeners.run_method(&Listener::on_all_downloads_finished);
}
void GlobalResourceDownloadManager::report_download_failed(const std::string& resource_slug){
    m_download_listeners.run_method(&Listener::on_download_failed, resource_slug);
}
void GlobalResourceDownloadManager::report_unexpected_exception_caught(const std::string& error_msg){
    m_download_listeners.run_method(&Listener::on_exception_caught, error_msg);
}

/////////////////////
// for ResourceDownload::Listener
/////////////////////

void GlobalResourceDownloadManager::on_download_finished(bool success, const std::string& resource_slug){
    remove_from_download_list(resource_slug);
    // check_if_all_downloads_done();
}
void GlobalResourceDownloadManager::on_download_failed(const std::string& resource_slug){
    report_download_failed(resource_slug);
}
void GlobalResourceDownloadManager::on_exception_caught(const std::string& error_msg){
    report_unexpected_exception_caught(error_msg);
}

}
