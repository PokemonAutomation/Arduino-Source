/*  Resource Download Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadManager_H
#define PokemonAutomation_ResourceDownloadManager_H

#include <vector>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/ListenerSet.h"
#include "ResourceDownload.h"

namespace PokemonAutomation{

// class ResourceDownload;



class GlobalResourceDownloadManager : public ResourceDownload::Listener{
public:
    struct Listener{
        virtual void on_all_downloads_finished(){}
        virtual void on_download_failed(const std::string& resource_slug){}
        virtual void on_exception_caught(const std::string& error_msg){}
    };

    void add_download_listener(Listener& listener);
    void remove_download_listener(Listener& listener);

    void report_all_downloads_finished();
    void report_download_failed(const std::string& resource_slug);
    void report_unexpected_exception_caught(const std::string& error_msg);

public:
    static GlobalResourceDownloadManager& instance();

public:  // ResourceDownload::Listener
    virtual void on_download_finished(bool success, const std::string& resource_slug) override;
    virtual void on_download_failed(const std::string& resource_slug) override;
    virtual void on_exception_caught(const std::string& error_msg) override;


public:
    // check if the download queue already has the resource
    // if not, add the download to the back of the download queue
    // and start downloading it
    // Also, add this manager as a listener to the download
    std::shared_ptr<ResourceDownload> add_to_download_list(const std::string& resource_slug);

    void remove_from_download_list(const std::string& resource_slug);

    // return true if given resource_slug's position in m_download_queue is less than MAX_CONCURRENT_DOWNLOADS
    // ASSUMES: the calling thread holds the m_lock. therefore, this function doesn't lock the mutex when accessing m_download_queue.
    bool is_download_ready_to_start(const std::string& resource_slug);


    void cancel_downloads();



private:

    // queue of downloads
    // ResourceDownload must be a pointer for several reasons:
    // - it contains a Mutex/CV, and so can't be moved unless it's a pointer. 
    // Objects can't be added to a vector if they can't be moved/copied.
    // - Other objects hold references to ResourceDownload. We will need to
    // remove items in the middle of the vector. This forces the vector to shift 
    // all subsequent elements forward in memory to fill the gap. This shifting 
    // breaks (invalidates) any references or iterators pointing to elements 
    // at or after the deletion point.
    std::vector<std::shared_ptr<ResourceDownload>> m_download_queue;

    Mutex m_queue_lock;
    ConditionVariable m_cv;

    ListenerSet<Listener> m_download_listeners;


};



}
#endif
