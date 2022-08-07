/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SwitchSystem.h"
#include "NintendoSwitch_SwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SwitchSystemSession::~SwitchSystemSession(){
    ProgramTracker::instance().remove_console(m_instance_id);
    m_factory.m_camera.info = m_camera->current_device();
    m_factory.m_camera.current_resolution = m_camera->current_resolution();
}
SwitchSystemSession::SwitchSystemSession(
    SwitchSystemFactory& factory,
    Logger& raw_logger,
    uint64_t program_id
)
    : m_logger(raw_logger, factory.m_logger_tag)
    , m_factory(factory)
    , m_serial(m_logger, factory.m_serial)
    , m_camera(get_camera_backend().make_camera(m_logger, DEFAULT_RESOLUTION))
    , m_audio(m_logger, factory.m_audio)
{
    m_camera->set_resolution(factory.m_camera.current_resolution);
    m_camera->set_source(factory.m_camera.info);
    m_instance_id = ProgramTracker::instance().add_console(program_id, *this);
}

BotBaseHandle& SwitchSystemSession::sender(){
    return m_serial.botbase();
}
VideoFeed& SwitchSystemSession::video(){
    return *m_camera;
}
AudioFeed& SwitchSystemSession::audio(){
    return m_audio;
}



}
}
