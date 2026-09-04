/*  Console System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state of an entire Switch system.
 *
 *    - Camera
 *    - Audio
 *    - Video Overlay
 *    - Controllers
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_ConsoleSystemSession_H
#define PokemonAutomation_ConsoleInfra_ConsoleSystemSession_H

#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Common/Cpp/UiWrapper.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "Integrations/ProgramTrackerInterfaces.h"
#include "ControllerInput/ControllerInput.h"
#include "Controllers/ControllerSession.h"
#include "ConsoleSystemOption.h"

namespace PokemonAutomation{
    class MemoryUtilizationStats;
    class CpuUtilizationStat;
    class ThreadUtilizationStat;
namespace ConsoleInfra{


class ConsoleSystemSession
    : public UiState<ConsoleSystemSession>
    , public TrackableConsole
    , private VideoDisplayHidListener
    , private ControllerInputListener
{
public:
    struct Listener{
        virtual void on_input_status_change(const std::string& status){}
        virtual void on_lock_controllers(){}
        virtual void on_unlock_controllers(){}
    };

    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }


public:
    virtual bool try_shutdown() noexcept;
    virtual ~ConsoleSystemSession();
    ConsoleSystemSession(
        Logger& logger,
        ConsoleSystemOption& option,
        size_t console_number,
        std::optional<uint64_t> program_tracking_id = {}
    );


public:
    size_t console_number() const{ return m_console_number; }
    bool allow_commands_while_locked() const{ return m_option.m_allow_commands_while_locked; }
    std::string status() const;

    Logger& logger(){ return m_logger; }

    VideoSession& video(){ return m_video; }
    AudioSession& audio(){ return m_audio; }
    VideoOverlaySession& overlay(){ return m_overlay; }
    const StreamHistorySession& stream_history() const{ return m_history; }

    size_t controllers() const{ return m_controllers.size(); }
    ControllerSession& controller(size_t index){ return m_controllers[index].session; }


public:
    virtual VideoFeed& video_feed() override{ return video(); }
    virtual AudioFeed& audio_feed() override{ return audio(); }
    virtual ControllerSession& controller() override{ return ConsoleSystemSession::controller(0); };

    virtual void save(ConsoleSystemOption& option) const;
    virtual void load(const ConsoleSystemOption& option);


public:
    void lock_controllers(std::string reason);
    void unlock_controllers();
    void save_history(const std::string& filename);


private:
    std::string update_status();

    virtual void on_focus_in() override;
    virtual void on_focus_out() override;

    virtual void run_controller_input(ControllerInputState& state) override;


private:
    //  The console # within a program.
    const size_t m_console_number;
    std::optional<uint64_t> m_console_tracking_id;

    TaggedLogger m_logger;
    ConsoleSystemOption& m_option;

    VideoSession m_video;
    AudioSession m_audio;
    VideoOverlaySession m_overlay;

    struct ControllerEntry{
        TaggedLogger logger;
        ControllerSession session;

        ControllerEntry(TaggedLogger& p_logger, ControllerOption& option)
            : logger(p_logger)
            , session(logger, option, std::nullopt)
        {}
        ControllerEntry(TaggedLogger& p_logger, size_t p_controller_index, ControllerOption& option)
            : logger(p_logger, std::to_string(p_controller_index))
            , session(logger, option, p_controller_index)
        {}
    };
    FixedLimitVector<ControllerEntry> m_controllers;

    StreamHistorySession m_history;

    mutable Mutex m_lock;
    std::string m_lock_controllers_reason;
    bool m_focused = false;
    std::string m_status_text;

    std::unique_ptr<MemoryUtilizationStats> m_memory_usage;
    std::unique_ptr<CpuUtilizationStat> m_cpu_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_main_thread_utilization;

    ListenerSet<Listener> m_listeners;
};





}
}
#endif
