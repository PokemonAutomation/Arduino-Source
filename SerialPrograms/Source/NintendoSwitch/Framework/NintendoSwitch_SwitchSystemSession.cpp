/*  Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/Backends/CameraImplementations.h"
#include "Integrations/ProgramTracker.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_SwitchSystemOption.h"
#include "NintendoSwitch_SwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SwitchSystemSession::~SwitchSystemSession(){
    ProgramTracker::instance().remove_console(m_instance_id);
    m_option.m_camera.info = m_camera->current_device();
    m_option.m_camera.current_resolution = m_camera->current_resolution();
}
SwitchSystemSession::SwitchSystemSession(
    SwitchSystemOption& option,
    uint64_t program_id
)
    : m_logger(global_logger_raw(), option.m_logger_tag)
    , m_option(option)
    , m_serial(m_logger, option.m_serial)
    , m_camera(get_camera_backend().make_camera(m_logger, DEFAULT_RESOLUTION))
    , m_audio(m_logger, option.m_audio)
{
    m_camera->set_resolution(option.m_camera.current_resolution);
    m_camera->set_source(option.m_camera.info);
    m_instance_id = ProgramTracker::instance().add_console(program_id, *this);
}

void SwitchSystemSession::set_allow_user_commands(bool allow){
    m_serial.botbase().set_allow_user_commands(allow);
}





}
}
