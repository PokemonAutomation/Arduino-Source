/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class holds the run-time state of an entire Switch system.
 *
 *    - Serial Port
 *    - Camera
 *    - Audio
 *    - Video Overlay
 *
 *  This class is fully thread-safe. You can call any functions from anywhere at
 *  anytime.
 *
 *  Warning: Constructing this class requires an "option" parameter. It is not
 *  safe to modify this "option" parameter during the lifetime of this class.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemSession_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemSession_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "Controllers/ControllerSession.h"
#include "Integrations/ProgramTrackerInterfaces.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
    class MemoryUtilizationStats;
    class CpuUtilizationStat;
    class ThreadUtilizationStat;
namespace NintendoSwitch{

class SwitchSystemOption;




class SwitchSystemSession final : public TrackableConsole{
public:
    ~SwitchSystemSession();
    SwitchSystemSession(
        SwitchSystemOption& option,
        uint64_t program_id,
        size_t console_number
    );

public:
    size_t console_number() const{ return m_console_number; }
    bool allow_commands_while_running() const{ return m_option.m_allow_commands_while_running; }

    Logger& logger(){ return m_logger; }
    virtual VideoFeed& video() override{ return m_video; }
    virtual AudioFeed& audio() override{ return m_audio; }
    virtual ControllerSession& controller() override{ return m_controller; };
    VideoOverlay& overlay(){ return m_overlay; }
    const StreamHistorySession& stream_history() const{ return m_history; }
    ConsoleModelCell& console_type(){ return m_option.m_console_type; }

public:
    void get(SwitchSystemOption& option);
    void set(const SwitchSystemOption& option);

    ControllerSession& controller_session(){ return m_controller; }
    VideoSession& video_session(){ return m_video; }
    AudioSession& audio_session(){ return m_audio; }
    VideoOverlaySession& overlay_session(){ return m_overlay; }

public:
    void set_allow_user_commands(std::string disallow_reason);
    void save_history(const std::string& filename);

private:
    //  The console # within a program.
    const size_t m_console_number;

    //  Globally unique ID.
    uint64_t m_console_id = 0;

    TaggedLogger m_logger;
    SwitchSystemOption& m_option;

    ControllerSession m_controller;
    VideoSession m_video;
    AudioSession m_audio;
    VideoOverlaySession m_overlay;

    StreamHistorySession m_history;

    std::unique_ptr<MemoryUtilizationStats> m_memory_usage;
    std::unique_ptr<CpuUtilizationStat> m_cpu_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_main_thread_utilization;
};



}
}
#endif
