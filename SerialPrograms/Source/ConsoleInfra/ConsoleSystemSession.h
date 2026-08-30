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
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "Controllers/ControllerSession.h"
#include "ConsoleSystemOption.h"

namespace PokemonAutomation{
    class MemoryUtilizationStats;
    class CpuUtilizationStat;
    class ThreadUtilizationStat;
namespace ConsoleInfra{


class ConsoleSystemSession{
public:
    virtual bool try_shutdown() noexcept;
    virtual ~ConsoleSystemSession();
    ConsoleSystemSession(
        Logger& logger,
        ConsoleSystemOption& option,
        size_t console_number
    );


public:
    size_t console_number() const{ return m_console_number; }
    bool allow_commands_while_running() const{ return m_option.m_allow_commands_while_running; }

    Logger& logger(){ return m_logger; }

    VideoSession& video(){ return m_video; }
    AudioSession& audio(){ return m_audio; }
    VideoOverlaySession& overlay(){ return m_overlay; }
    const StreamHistorySession& stream_history() const{ return m_history; }

    size_t controllers() const{ return m_controllers.size(); }
    ControllerSession& controller(size_t index) const{ return *m_controllers[index]; }


public:
    virtual void get(ConsoleSystemOption& option) const;
    virtual void set(const ConsoleSystemOption& option);


public:
    void set_allow_user_commands(const std::string& disallow_reason);
    void save_history(const std::string& filename);


private:
    //  The console # within a program.
    const size_t m_console_number;

    TaggedLogger m_logger;
    ConsoleSystemOption& m_option;

    VideoSession m_video;
    AudioSession m_audio;
    VideoOverlaySession m_overlay;
    std::vector<std::unique_ptr<ControllerSession>> m_controllers;

    StreamHistorySession m_history;

    std::unique_ptr<MemoryUtilizationStats> m_memory_usage;
    std::unique_ptr<CpuUtilizationStat> m_cpu_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_main_thread_utilization;
};





}
}
#endif
