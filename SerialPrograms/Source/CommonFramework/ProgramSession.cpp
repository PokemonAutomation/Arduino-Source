/*  Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <future>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PanicDump.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "CommonFramework/ProgramSession.h"
#include "CommonFramework/ProgramStats/StatsDatabase.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ResourceDownload/ProgramMissingResourceTracker.h"
#include "CommonFramework/ResourceDownload/GlobalResourceDownloadManager.h"
#include "CommonFramework/ResourceDownload/ResourceDownloadHelpers.h"
#include "Integrations/ProgramTracker.h"

namespace PokemonAutomation{



void ProgramSession::add_listener(Listener& listener){
    m_listeners.add(listener);
}
void ProgramSession::remove_listener(Listener& listener){
    m_listeners.remove(listener);
}



ProgramSession::ProgramSession(const ProgramDescriptor& descriptor)
    : m_descriptor(descriptor)
    , m_instance_id(ProgramTracker::instance().add_program(*this))
//    , m_logger(global_logger_raw(), "Program:" + std::to_string(m_instance_id))
    , m_logger(global_logger_raw(), "Program")
    , m_timestamp(current_time())
    , m_state(ProgramState::STOPPED)
{
    load_historical_stats();
}
ProgramSession::~ProgramSession(){
    ProgramTracker::instance().remove_program(m_instance_id);
//    ProgramSession::request_program_stop();
//    join_program_thread();
}

void ProgramSession::join_program_thread() noexcept{
    m_program_thread.wait_and_ignore_exceptions();
}



const std::string& ProgramSession::identifier() const{
    return m_descriptor.identifier();
}
std::string ProgramSession::current_stats() const{
    std::lock_guard<Mutex> lg(m_lock);
    if (m_current_stats){
        return m_current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN);
    }
    return "";
}
std::string ProgramSession::historical_stats() const{
    std::lock_guard<Mutex> lg(m_lock);
    if (m_historical_stats){
        return m_historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN);
    }
    return "";
}
WallClock ProgramSession::timestamp() const{
    return m_timestamp.load(std::memory_order_relaxed);
}


void ProgramSession::report_stats_changed(){
    std::lock_guard<Mutex> lg(m_lock);
    push_stats();
}
void ProgramSession::report_error(const std::string& message){
    std::lock_guard<Mutex> lg(m_lock);
    push_error(message);
}

void ProgramSession::report_download_error(const std::string& message){
    std::lock_guard<Mutex> lg(m_lock);
    push_download_error(message);
}

void ProgramSession::report_download_added(std::shared_ptr<ResourceDownload> download_ptr){
    // std::lock_guard<Mutex> lg(m_lock);
    m_listeners.run_method(&Listener::download_added, std::move(download_ptr));
}

void ProgramSession::report_all_downloads_done(){
    m_listeners.run_method(&Listener::all_downloads_done);
}


void ProgramSession::set_state(ProgramState state){
    switch (state){
    case ProgramState::NOT_READY:
        m_logger.log("Program State: NOT_READY");
        break;
    case ProgramState::STOPPED:
        m_logger.log("Program State: STOPPED");
        break;
    case ProgramState::RUNNING:
        m_logger.log("Program State: RUNNING");
        break;
    case ProgramState::STOPPING:
        m_logger.log("Program State: STOPPING");
        break;
    }
    m_state.store(state, std::memory_order_relaxed);
    m_listeners.run_method(&Listener::state_change, state);
}
void ProgramSession::push_stats(){
    m_listeners.run_method(
        &Listener::stats_update,
        m_current_stats.get(),
        m_historical_stats.get()
    );
}
void ProgramSession::push_error(const std::string& message){
    m_listeners.run_method(&Listener::error, message);
}

void ProgramSession::push_download_error(const std::string& message){
    m_listeners.run_method(&Listener::download_error, message);
}


void ProgramSession::load_historical_stats(){
    //  Load historical stats.
    std::unique_ptr<StatsTracker> stats = m_descriptor.make_stats();
    if (stats){
        m_logger.log("Loading historical stats...");
//        m_current_stats = m_descriptor.make_stats();
        StatSet set;
        set.open_from_file(GlobalSettings::instance().STATS_FILE);
        const std::string& identifier = m_descriptor.identifier();
        StatList& list = set[identifier];
        if (list.size() != 0){
            list.aggregate(*stats);
        }
        m_historical_stats = std::move(stats);
    }
}
void ProgramSession::update_historical_stats_with_current(){
    if (m_current_stats){
        m_logger.log("Saving historical stats...");
        bool ok = StatSet::update_file(
            GlobalSettings::instance().STATS_FILE,
            m_descriptor.identifier(),
            *m_current_stats
        );
        if (ok){
            m_logger.log("Stats successfully saved!", COLOR_BLUE);
        }else{
            m_logger.log("Unable to save stats.", COLOR_RED);
            push_error("Unable to save stats.");
        }
    }
}


std::string ProgramSession::start_program(){
    m_logger.log("Received Program Start Request");
    std::lock_guard<Mutex> lg(m_lock);

    ProgramState state = this->current_state();
    switch (state){
    case ProgramState::NOT_READY:
        m_logger.log("Program is not ready.", COLOR_RED);
        return "Program is not ready.";

    case ProgramState::STOPPED:{
        std::string error = check_validity();
        if (!error.empty()){
            m_logger.log(error, COLOR_RED);
            return error;
        }

        //  Wait for previous program thread to finish.
        join_program_thread();

        //  Now start the program.
        m_logger.log("Starting program...");
        m_timestamp.store(current_time(), std::memory_order_relaxed);
        set_state(ProgramState::RUNNING);
        m_program_thread = GlobalThreadPools::unlimited_realtime().dispatch_now_blocking(
            [this]{
                run_with_catch(
                    "ProgramSession::start_program()",
                    [this]{ run_program(); }
                );
            }
        );
#if 0
        m_thread = Thread([this]{
            run_with_catch(
                "ProgramSession::start_program()",
                [this]{ run_program(); }
            );
        });
#endif
        return "";
    }
    case ProgramState::RUNNING:
        m_logger.log("Program is already running.", COLOR_RED);
        return "Program is already running.";

    case ProgramState::STOPPING:
        m_logger.log("Program is currently stopping.", COLOR_RED);
        return "Program is currently stopping.";
    }

    throw InternalProgramError(
        &m_logger,
        PA_CURRENT_FUNCTION,
        "Invalid State: " + std::to_string((int)state)
    );
}
std::string ProgramSession::stop_program(){
    m_logger.log("Received Stop Request");
    {
        std::lock_guard<Mutex> lg(m_lock);

        ProgramState state = this->current_state();
        switch (state){
        case ProgramState::NOT_READY:
            m_logger.log("Program is not ready.", COLOR_RED);
            return "Program is not ready.";
        case ProgramState::STOPPED:
            m_logger.log("Program is already stopped.", COLOR_RED);
            return "Program is already stopped.";
        case ProgramState::RUNNING:
            m_logger.log("Stopping program...");
            set_state(ProgramState::STOPPING);
            break;
        case ProgramState::STOPPING:
            m_logger.log("Program is already stopping.", COLOR_RED);
            return "Program is already stopping.";
        default:
            throw InternalProgramError(
                &m_logger,
                PA_CURRENT_FUNCTION,
                "Invalid State: " + std::to_string((int)state)
            );
        }
    }
    internal_stop_program();
    return "";
}



void ProgramSession::run_program(){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_current_stats = m_descriptor.make_stats();
        load_historical_stats();
        push_stats();
    }
    internal_run_program();
    {
        std::lock_guard<Mutex> lg(m_lock);
        push_stats();
        update_historical_stats_with_current();
        set_state(ProgramState::STOPPED);
    }
}



RequiredResourceResult ProgramSession::find_missing_resources(){

    bool upgrade_warning = false;

    std::vector<std::string> missing_resources;
    for(const std::string& resource_type : m_descriptor.required_resources()){
        ResourceVersionStatus version_status = get_local_version_info(resource_type).version_status;

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
            DownloadedResourceMetadata remote_resource = get_remote_resource_metadata_from_resource_slug(resource_type);
            DownloadedResourceMetadata expected_resource = get_expected_resource_metadata_from_resource_slug(resource_type);
            uint16_t expected_version_num = expected_resource.version_num.value();
            uint16_t remote_version_num = remote_resource.version_num.value();

            if (expected_version_num < remote_version_num){
                // remote version is more updated than we expect
                // warn the user to upgrade CC.
                upgrade_warning = true;

            }else if (expected_version_num > remote_version_num){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "resources_to_download: expected_version_num > remote_version_num. This shouldn't happen."); 
            }

            missing_resources.emplace_back(remote_resource.resource_name);

            break;
        }
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "resources_to_download: Unknown enum."); 
        }
    }


    return RequiredResourceResult{std::move(missing_resources), upgrade_warning};

}



bool ProgramSession::download_prereqs(CancellableScope& scope){

    try{

        auto [missing_resources, requires_upgrade] = find_missing_resources();

        if (requires_upgrade){ 
            std::string warning_string = 
                "The program is expecting an older version of a resource than is available. "
                "This likely means that your version of Computer Control is out of date. "
                "We recommend that you upgrade the Computer Control program.";
            report_error(warning_string);
            // cout << warning_string << endl;
        }
        if (missing_resources.empty()){
            // cout << "required_download_list is empty. Start the program." << endl;

            return true;
        }

        // 1. Create a C++ standard promise to hold the final boolean result
        std::promise<bool> download_promise;
        std::future<bool> download_future = download_promise.get_future();

        // ProgramMissingResourceTracker is responsible for calling report_error() if
        // the download fails, or if exceptions are thrown within the download.
        ProgramMissingResourceTracker missing_resource_tracker(scope, download_promise, *this);

        GlobalResourceDownloadManager& global_download_manager = GlobalResourceDownloadManager::instance();
        for (const std::string& resource_slug : missing_resources){
            // cout << download_ptr->get_name() << endl;

            auto download_ptr = global_download_manager.add_to_download_list(resource_slug);
            if (!download_ptr) {
                std::cerr << "Error: Null download pointer for " << resource_slug << std::endl;
                continue;
            }
            missing_resource_tracker.add_resource(download_ptr);
            report_download_added(download_ptr);
        }
        missing_resource_tracker.finalize_initial_batch();

        // Block until the ProgramMissingResourceTracker calls set_value(), 
        // when the downloads all succeed, or one fails, or the user clicks "Stop Program"
        bool success = download_future.get();

        report_all_downloads_done();

        return success;
   
            
    }catch(OperationFailedException& e){
        report_error(e.message());
    }catch(InternalProgramError& e){
        report_error(e.message());
    }catch (const std::exception& e) {
        std::string message = std::string(e.what()) + "Report this as an error.";
        report_error(message);
    }catch(...){
        report_error("show_download_prereqs_popup: Unknown exception caught. Report this as an error.");
    }

    return false;

}








}
