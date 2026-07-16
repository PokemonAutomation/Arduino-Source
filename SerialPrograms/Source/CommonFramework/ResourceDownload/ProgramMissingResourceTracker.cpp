/*  ProgramMissingResourceTracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


// #include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ProgramSession.h"
#include "ProgramMissingResourceTracker.h"


#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ProgramMissingResourceTracker::~ProgramMissingResourceTracker(){
    m_scope.remove_cancel_listener(*this);
    for (const auto& resource : m_missing_resources){
        resource->remove_listener(*this);
    }
}

ProgramMissingResourceTracker::ProgramMissingResourceTracker(CancellableScope& scope, std::promise<bool>& p, ProgramSession& session)
    : m_scope(scope)
    , m_promise(p)
    , m_session(session)
{
    scope.add_cancel_listener(*this);
}

void ProgramMissingResourceTracker::add_resource(const std::shared_ptr<ResourceDownload>& resource){
    std::lock_guard<Mutex> lg(m_lock);
    
    try {
        m_missing_resources.insert(resource);
        // throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "test."); 
        resource->add_listener(*this);
    }catch(...){
        m_missing_resources.erase(resource);
        resource->cancel_download();
        throw;
    }

}


void ProgramMissingResourceTracker::remove_resource(const std::string& resource_slug){
    std::lock_guard<Mutex> lg(m_lock);

    // this requires C++20
    std::erase_if(m_missing_resources, [&](const auto& download_ptr) {
        return download_ptr->get_name() == resource_slug;
    });

}

void ProgramMissingResourceTracker::finalize_initial_batch() {
    std::lock_guard<Mutex> lg(m_lock);
    m_is_accepting_jobs = false;

    // Handles the edge case where everything finished before we even stopped adding jobs
    if (m_missing_resources.empty()) {
        if (!m_failed) {
            fulfill_promise(true);
        }
    }
}

void ProgramMissingResourceTracker::check_if_all_downloads_done(){
    std::lock_guard<Mutex> lg(m_lock);

    // check if m_missing_resources is empty and that we are no longer accepting new jobs
    // checking m_is_accepting_jobs covers for the case where Download 1 finishes before Download 2 is added. 
    // this prevents the completion event from firing prematurely
    if (m_missing_resources.empty() && !m_is_accepting_jobs){
        if (!m_failed) {
            fulfill_promise(true);
        }
    }
}

void ProgramMissingResourceTracker::cancel_downloads(){
    std::unordered_set<std::shared_ptr<ResourceDownload>> downloads_to_cancel;

    // Lock briefly to safely snapshot the queue
    {
        std::lock_guard<Mutex> lg(m_lock);
        downloads_to_cancel = m_missing_resources; 
    }

    // We must call cancel_download() outside of the lock.
    // The reason is that ProgramMissingResourceTracker is a listener to ResourceDownload. 
    // ResourceDownload::cancel_download() will trigger ProgramMissingResourceTracker::on_download_finished,
    // which then triggers ProgramMissingResourceTracker::remove_resource(), which is 
    // done under a lock.
    // This prevents a deadlock.
    for (auto& download : downloads_to_cancel) {
        if (download) {
            download->cancel_download(); 
        }
    }

}

void ProgramMissingResourceTracker::fulfill_promise(bool value) {
    std::call_once(m_fulfill_flag, [this, value]() {
        m_promise.set_value(value);
    });
}

//////////////////////////
// TrackerListener
//////////////////////////

// void ProgramMissingResourceTracker::add_tracker_listener(TrackerListener& listener){
//     m_tracker_listeners.add(listener);
// }
// void ProgramMissingResourceTracker::remove_tracker_listener(TrackerListener& listener){
//     m_tracker_listeners.remove(listener);
// }

// void ProgramMissingResourceTracker::report_all_downloads_finished(){
//     m_tracker_listeners.run_method(&TrackerListener::on_all_missing_downloads_finished);
// }

// void ProgramMissingResourceTracker::report_download_failed(const std::string& resource_slug){
//     m_tracker_listeners.run_method(&TrackerListener::on_download_failed, resource_slug);
// }

// void ProgramMissingResourceTracker::report_unexpected_exception_caught(const std::string& error_msg){
//     m_tracker_listeners.run_method(&TrackerListener::on_unexpected_exception_caught, error_msg);
// }

/////////////////////
// for ResourceDownload::Listener
/////////////////////
void ProgramMissingResourceTracker::on_download_finished(bool success, const std::string& resource_slug){
    // cout << "ProgramMissingResourceTracker::on_download_finished " << resource_slug << endl;
    remove_resource(resource_slug);
    if (success){
        check_if_all_downloads_done();
    }
}
void ProgramMissingResourceTracker::on_download_failed(const std::string& resource_slug){
    m_failed = true;
    std::cerr << "ProgramMissingResourceTracker::on_download_failed: Error: Download failed for " << resource_slug << ". Check your internet connection and check you have enough disk space." << std::endl;
    m_session.report_download_error("Error: Download failed for " + resource_slug + ". Check your internet connection and check you have enough disk space.");
    fulfill_promise(false);
}
void ProgramMissingResourceTracker::on_exception_caught(const std::string& error_msg){
    m_failed = true;
    std::cerr << "ProgramMissingResourceTracker::on_download_failed: Error: " << error_msg << std::endl;
    m_session.report_download_error(error_msg);
    fulfill_promise(false);
}

/////////////////////
// for Cancellable::CancelListener
/////////////////////
void ProgramMissingResourceTracker::on_cancellable_cancel(
    Cancellable& cancellable,
    std::exception_ptr reason
){
    m_failed = true;
    cancel_downloads();
    fulfill_promise(false);

}


}
