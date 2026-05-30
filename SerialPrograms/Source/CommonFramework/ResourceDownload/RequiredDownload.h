/*  Required Download
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_RequiredDownload_H
#define PokemonAutomation_RequiredDownload_H

#include <string>
// #include "ComputerPrograms/Framework/ComputerProgramSession.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/ListenerSet.h"
#include "DownloadThread.h"
#include "ResourceDownloadHelpers.h"


namespace PokemonAutomation{


class RequiredDownloadManager;

class RequiredDownload {
public:
    RequiredDownload(RequiredDownloadManager& download_manager, DownloadedResourceMetadata resource_metadata, Mutex& lock, ConditionVariable& cv);

public:
    inline std::string get_name() const { return m_name; }

public: 
    struct Listener{
        virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes){}
        virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){}
        virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes){}

        // virtual void on_exception_caught(const std::string& function_name){}
        virtual void on_download_failed(){}

    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);    

    void start_download();
    void cancel_download();

    void on_download_finished();
    // bool is_download_ready_to_start();
    // void remove_self_from_download_queue();

    void report_unexpected_exception_caught(const std::string& function_name);
    void report_download_failed();

    void report_download_progress(uint64_t bytes_done, uint64_t total_bytes);
    void report_unzip_progress(uint64_t bytes_done, uint64_t total_bytes);
    void report_hash_progress(uint64_t bytes_done, uint64_t total_bytes);

private:
    DownloadThread initialize_download_thread();

private:
    RequiredDownloadManager& m_download_manager;
    DownloadedResourceMetadata m_resource_metadata;

    std::string m_name;
    Mutex& m_lock;
    ConditionVariable& m_cv;
    DownloadThread m_download_thread;

    ListenerSet<Listener> m_listeners;

};



}
#endif
