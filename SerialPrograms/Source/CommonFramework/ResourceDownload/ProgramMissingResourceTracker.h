/*  ProgramMissingResourceTracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_MissingResourceTracker_H
#define PokemonAutomation_MissingResourceTracker_H

#include <unordered_set>
#include <future>
#include "Common/Cpp/Concurrency/Mutex.h"
// #include "Common/Cpp/ListenerSet.h"
#include "ResourceDownload.h"

namespace PokemonAutomation{

class ProgramSession;

class ProgramMissingResourceTracker : public ResourceDownload::Listener, public Cancellable::CancelListener {

public:
    ~ProgramMissingResourceTracker();
    ProgramMissingResourceTracker(CancellableScope& scope, std::promise<bool>& p, ProgramSession& session);

    // struct TrackerListener{
    //     virtual void on_all_missing_downloads_finished(){}
    //     virtual void on_download_failed(const std::string& resource_slug){}
    //     virtual void on_unexpected_exception_caught(const std::string& error_msg){}
    // };

    // void add_tracker_listener(TrackerListener& listener);
    // void remove_tracker_listener(TrackerListener& listener);

    // void report_all_downloads_finished();
    // void report_download_failed(const std::string& resource_slug);
    // void report_unexpected_exception_caught(const std::string& error_msg);

public:  // ResourceDownload::Listener
    // Remove the corresponding Resource from m_missing_resources
    // if the download was a success, run check_if_all_downloads_done()
    virtual void on_download_finished(bool success, const std::string& resource_slug) override;
    
    virtual void on_download_failed(const std::string& resource_slug) override;
    virtual void on_exception_caught(const std::string& error_msg) override;

public:  // Cancellable::CancelListener
    virtual void on_cancellable_cancel(
        Cancellable& cancellable,
        std::exception_ptr reason
    ) override;


public:
    void add_resource(const std::shared_ptr<ResourceDownload>& resource);
    void remove_resource(const std::string& resource_slug);
    void finalize_initial_batch();
    void check_if_all_downloads_done();
    void cancel_downloads();

private:
    void fulfill_promise(bool value);


private:
    CancellableScope& m_scope;
    std::promise<bool>& m_promise;
    ProgramSession& m_session;

    std::unordered_set<std::shared_ptr<ResourceDownload>> m_missing_resources;

    bool m_is_accepting_jobs = true;
    Mutex m_lock;
    // ListenerSet<TrackerListener> m_tracker_listeners;

    std::atomic<bool> m_failed = false;
    std::once_flag m_fulfill_flag;


};
}
#endif
