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
#include "CommonFramework/VideoPipeline/CameraSession.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "NintendoSwitch_SwitchSystem.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSystemFactory;




class SwitchSystemSession final : public ConsoleSystem{
public:
    ~SwitchSystemSession();
    SwitchSystemSession(
        SwitchSystemFactory& factory,
        Logger& raw_logger,
        uint64_t program_id
    );

public:
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

private:
    uint64_t m_instance_id = 0;
    TaggedLogger m_logger;
    SwitchSystemFactory& m_factory;

    SerialPortSession m_serial;
    std::unique_ptr<CameraSession> m_camera;
    AudioSession m_audio;
    VideoOverlaySession m_overlay;
};



}
}
#endif
