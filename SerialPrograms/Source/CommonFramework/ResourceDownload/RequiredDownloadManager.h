/*  Required Download Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_RequiredDownloadManager_H
#define PokemonAutomation_RequiredDownloadManager_H

#include <vector>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/ListenerSet.h"
// #include "CommonFramework/ResourceDownload/RequiredDownload.h"

namespace PokemonAutomation{

class RequiredDownload;

struct RequiredResourceResult {
    std::vector<std::shared_ptr<RequiredDownload>> downloads;
    bool requires_upgrade = false;
};

class RequiredDownloadManager {
public:
    struct DownloadListener{
        virtual void on_all_downloads_finished(){}
        virtual void on_download_failed(){}
        virtual void on_exception_caught(const std::string& function_name){}
    };

    void add_download_listener(DownloadListener& listener);
    void remove_download_listener(DownloadListener& listener);

    void report_all_downloads_finished();
    void report_download_failed();
    void report_unexpected_exception_caught(const std::string& function_name);

public:
    RequiredDownloadManager(const std::vector<std::string>& required_resources);

public:
    void initialize_required_downloads();

    bool get_upgrade_warning();

    const std::vector<std::shared_ptr<RequiredDownload>>& get_required_downloads();

    // void add_to_download_list(uint16_t resource_index);
    void remove_from_download_list(uint16_t resource_index);

    // return true if given resource_index's position in m_download_queue is less than MAX_CONCURRENT_DOWNLOADS
    // ASSUMES: the calling thread holds the m_lock. therefore, this function doesn't lock the mutex when accessing m_download_queue.
    bool is_download_ready_to_start(uint16_t resource_index);

    // check if the m_download_queue is empty
    // if so, trigger report_all_downloads_finished()
    void check_if_all_downloads_done();

    void cancel_downloads();

private:
    // find the list of resources that we need, but are not already downloaded. 
    // return struct that contains required_downloads and upgrade_warning.
    // required_downloads: the list of required downloads
    // upgrade_warning: is true if we need to warn the user to upgrade CC
    RequiredResourceResult find_resources_to_download(const std::vector<std::string>& required_resources);

private:
    std::vector<std::string> m_required_resources;

    bool m_resource_list_initialized{false};

    bool m_upgrade_warning;

    // RequiredDownload must be a pointer because it contains a Mutex/CV, 
    // and so can't be moved unless it's a pointer. 
    // Objects can't be added to a vector if they can't be moved/copied.
    // also, it being a pointer allows it to be forward declared. 
    // this is needed since RequiredDownload also holds a reference to this Session
    std::vector<std::shared_ptr<RequiredDownload>> m_required_downloads;  

    // queue of downloads
    // each download is represented by an int, 
    // which is their index within the local resource list. 
    std::vector<uint16_t> m_download_queue;

    Mutex m_queue_lock;
    ConditionVariable m_cv;

    ListenerSet<DownloadListener> m_download_listeners;
};

}
#endif
