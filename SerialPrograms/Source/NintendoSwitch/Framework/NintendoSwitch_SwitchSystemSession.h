/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "CommonFramework/ControllerDevices/SerialPortSession.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/CameraSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "Integrations/ProgramTrackerInterfaces.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
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
    PABotBaseLevel min_pabotbase() const{ return m_serial.min_pabotbase(); }
    size_t console_number() const{ return m_console_number; }
    bool allow_commands_while_running() const{ return m_option.m_allow_commands_while_running; }

    Logger& logger(){ return m_logger; }
    virtual BotBaseHandle& sender() override{ return m_serial.botbase(); }
    virtual VideoFeed& video() override{ return *m_camera; }
    virtual AudioFeed& audio() override{ return m_audio; }
    VideoOverlay& overlay(){ return m_overlay; }
    const StreamHistorySession& stream_history() const{ return m_history; }

public:
    void get(SwitchSystemOption& option);
    void set(const SwitchSystemOption& option);

    SerialPortSession& serial_session(){ return m_serial; }
    CameraSession& camera_session(){ return *m_camera; }
    AudioSession& audio_session(){ return m_audio; }
    VideoOverlaySession& overlay_session(){ return m_overlay; }

public:
    void set_allow_user_commands(bool allow);
    void save_history(const std::string& filename);

private:
    //  The console # within a program.
    const size_t m_console_number;

    //  Globally unique ID.
    uint64_t m_console_id = 0;

    TaggedLogger m_logger;
    SwitchSystemOption& m_option;

    SerialPortSession m_serial;
    std::unique_ptr<CameraSession> m_camera;
    AudioSession m_audio;
    VideoOverlaySession m_overlay;

    StreamHistorySession m_history;

    std::unique_ptr<CpuUtilizationStat> m_cpu_utilization;
    std::unique_ptr<ThreadUtilizationStat> m_main_thread_utilization;
};



}
}
#endif
