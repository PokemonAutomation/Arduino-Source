/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  This class holds the real-time state of an entire Switch system.
 *
 *    - Serial Port
 *    - Camera
 *    - Audio
 *    - Video Overlay
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SwitchSystemSession_H
#define PokemonAutomation_NintendoSwitch_SwitchSystemSession_H

#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ControllerDevices/SerialPortSession.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/CameraSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "Integrations/ProgramTrackerInterfaces.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSystemOption;




class SwitchSystemSession final : public TrackableConsole{
public:
    ~SwitchSystemSession();
    SwitchSystemSession(
        SwitchSystemOption& option,
        uint64_t program_id
    );

public:
    size_t console_id() const{ return m_option.m_console_id; }
    bool allow_commands_while_running() const{ return m_option.m_allow_commands_while_running; }

    Logger& logger(){ return m_logger; }
    virtual BotBaseHandle& sender() override{ return m_serial.botbase(); }
    virtual VideoFeed& video() override{ return *m_camera; }
    virtual AudioFeed& audio() override{ return m_audio; }
    VideoOverlay& overlay(){ return m_overlay; }

public:
    SerialPortSession& serial_session(){ return m_serial; }
    CameraSession& camera_session(){ return *m_camera; }
    AudioSession& audio_session(){ return m_audio; }
    VideoOverlaySession& overlay_session(){ return m_overlay; }

public:
    void set_allow_user_commands(bool allow);

private:
    uint64_t m_instance_id = 0;
    TaggedLogger m_logger;
    SwitchSystemOption& m_option;

    SerialPortSession m_serial;
    std::unique_ptr<CameraSession> m_camera;
    AudioSession m_audio;
    VideoOverlaySession m_overlay;
};



}
}
#endif
