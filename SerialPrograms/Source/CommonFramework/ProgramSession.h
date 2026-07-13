/*  Program Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      ProgramSession represents the real-time state of a program.
 *
 *  It holds the state machine whether it's running/stopping, etc... It also
 *  holds misc. data that is common to all programs.
 *
 *  As with all "Session" classes, this class is fully thread-safe and is
 *  not responsible for any UI. UI classes must attach themselves to a
 *  session to receive updates.
 *
 */

#ifndef PokemonAutomation_CommonFramework_ProgramSession_H
#define PokemonAutomation_CommonFramework_ProgramSession_H

#include <atomic>
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Common/Cpp/Time.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/Globals.h"
//#include "CommonFramework/Logging/Logger.h"
// #include "CommonFramework/ResourceDownload/ProgramMissingResourceTracker.h"
#include "Integrations/ProgramTrackerInterfaces.h"

namespace PokemonAutomation{

class StatsTracker;
class CancellableScope;
class ProgramDescriptor;
class ResourceDownload;

struct RequiredResourceResult {
    std::vector<std::string> missing_resources;
    bool requires_upgrade = false;
};


class ProgramSession : public TrackableProgram{
public:
    struct Listener{
        virtual void state_change(ProgramState state) = 0;
        virtual void stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats) = 0;
        virtual void error(const std::string& message) = 0;
        virtual void download_error(const std::string& message) = 0;
        virtual void download_added(std::shared_ptr<ResourceDownload> download_ptr) = 0;
        virtual void all_downloads_done() = 0;
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    //  Setup

    ProgramSession(const ProgramDescriptor& descriptor);

    //  Before the child-most class begins destruction, you must first:
    //      1.  Stop the program.
    //      2.  Join the program thread.
    virtual ~ProgramSession();

    const ProgramDescriptor& descriptor() const{ return m_descriptor; }
    uint64_t instance_id() const{ return m_instance_id; }
    Logger& logger(){ return m_logger; }


    // - return struct that contains the list of missing resources and boolean upgrade_warning.
    // - upgrade_warning: is true if we need to warn the user to upgrade CC. 
    // i.e. the resource that was/will be downloaded is more updated than what the program is expecting
    // - a resource is considered to be missing if its ResourceVersionStatus is OUTDATED or NOT_APPLICABLE.
    // - throw OperationFailedException if one of the required_resources isn't found within remote_resource_download_list
    RequiredResourceResult find_missing_resources();

    bool download_prereqs(CancellableScope& scope);


public:
    //  Getters

    virtual const std::string& identifier() const override final;
    virtual ProgramState current_state() const override final{ return m_state.load(std::memory_order_relaxed); }
    virtual std::string current_stats() const override final;
    std::string historical_stats() const;
    virtual WallClock timestamp() const final;

    //  Temporary for migration.
    StatsTracker* current_stats_tracker(){ return m_current_stats.get(); }
    const StatsTracker* historical_stats_tracker() const{ return m_historical_stats.get(); }

    Mutex& program_lock() const{ return m_lock; }


public:
    //  Program Control

    //  Returns empty string on success. Otherwise returns error string.
    //  Note that these are asynchronous and will return before they are finished.

    std::string start_program();    //  Returns before the program is fully started up.
    std::string stop_program();     //  Returns before the program is completely stopped.

    //  Temporary
    virtual void async_start() override{ start_program(); }
    virtual void async_stop() override{ stop_program(); }


protected:
    virtual void internal_run_program() = 0;
    virtual void internal_stop_program() = 0;

//    virtual void restore_defaults(){ return; }

public:
    void report_stats_changed();
    void report_error(const std::string& message);
    void report_download_error(const std::string& message);
    void report_download_added(std::shared_ptr<ResourceDownload> download_ptr);
    void report_all_downloads_done();


protected:
    //  Everything here must be called under the lock.

    virtual std::string check_validity() const{ return ""; }
    void join_program_thread() noexcept;

private:
    //  Everything here must be called under the lock.

    void set_state(ProgramState state);
    void push_stats();
    void push_error(const std::string& message);
    void push_download_error(const std::string& message);
    void load_historical_stats();
    void update_historical_stats_with_current();


private:
    void run_program();


private:
    const ProgramDescriptor& m_descriptor;

    uint64_t m_instance_id = 0;
    TaggedLogger m_logger;

    mutable Mutex m_lock;

    std::atomic<WallClock> m_timestamp;
    std::atomic<ProgramState> m_state;

    // ProgramMissingResourceTracker m_missing_resource_tracker;

    AsyncTask m_program_thread;

//    Mutex m_stats_lock;
    std::unique_ptr<StatsTracker> m_historical_stats;
    std::unique_ptr<StatsTracker> m_current_stats;
//    CancellableScope* m_scope = nullptr;

    ListenerSet<Listener> m_listeners;
};



}
#endif
