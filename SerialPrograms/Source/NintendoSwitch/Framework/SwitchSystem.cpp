/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Common/Qt/QtJsonTools.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "Integrations/ProgramTracker.h"
#include "SwitchCommandRow.h"
#include "SwitchSystem.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


const QString SwitchSystemFactory::JSON_SERIAL  = "Serial";
const QString SwitchSystemFactory::JSON_CAMERA  = "Camera";
const QString SwitchSystemFactory::JSON_CAMERA_INDEX        = "CameraIndex";
const QString SwitchSystemFactory::JSON_CAMERA_RESOLUTION   = "CameraResolution";



SwitchSystemFactory::SwitchSystemFactory(
    QString label, std::string logger_tag,
    PABotBaseLevel min_pabotbase, FeedbackType feedback
)
    : SwitchSetupFactory(min_pabotbase, feedback)
    , m_label(std::move(label))
    , m_logger_tag(std::move(logger_tag))
    , m_serial("<b>Serial Port:</b>", m_logger_tag, min_pabotbase)
    , m_camera(std::move(label))
{}
SwitchSystemFactory::SwitchSystemFactory(
    QString label, std::string logger_tag,
    PABotBaseLevel min_pabotbase, FeedbackType feedback,
    const QJsonValue& json
)
    : SwitchSystemFactory(
          std::move(label),
          std::move(logger_tag),
          min_pabotbase, feedback
     )
{
    load_json(json);
}
void SwitchSystemFactory::load_json(const QJsonValue& json){
    QJsonObject obj = json.toObject();
    m_serial.load_json(json_get_value_nothrow(obj, JSON_SERIAL));
    m_camera.load_json(json_get_value_nothrow(obj, JSON_CAMERA));
}
QJsonValue SwitchSystemFactory::to_json() const{
    QJsonObject root;
    root.insert(JSON_SERIAL, m_serial.to_json());
    root.insert(JSON_CAMERA, m_camera.to_json());
    return root;
}

const QSerialPortInfo* SwitchSystemFactory::port() const{
    return m_serial.port();
}

SwitchSetup* SwitchSystemFactory::make_ui(QWidget& parent, Logger& logger, uint64_t program_id){
    return new SwitchSystem(parent, *this, logger, program_id);
}



SwitchSystem::SwitchSystem(
    QWidget& parent,
    SwitchSystemFactory& factory,
    Logger& logger,
    uint64_t program_id
)
    : SwitchSetup(parent, factory)
    , m_factory(factory)
    , m_logger(logger, factory.m_logger_tag)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    m_serial = factory.m_serial.make_ui(*this, logger);
    layout->addWidget(m_serial);

    QWidget* video = new QWidget(this);
    QHBoxLayout* video_layout = new QHBoxLayout(video);
    video_layout->setMargin(0);

    m_camera = factory.m_camera.make_ui(*this, logger, *video);
    layout->addWidget(m_camera);

    m_command = new CommandRow(
        *this,
        m_serial->botbase(),
        m_logger,
        factory.m_feedback
    );
    layout->addWidget(m_command);

    layout->addWidget(video);
    m_camera->reset_video();

//    m_controller.reset(new VirtualController(m_serial->botbase()));
    setFocusPolicy(Qt::StrongFocus);


    connect(
        m_serial, &SerialSelectorUI::on_ready,
        m_command, [=](bool ready){
            m_command->update_ui();
        }
    );
    connect(
        m_command, &CommandRow::set_feedback_enabled,
        m_camera, [=](bool enabled){
            m_camera->set_snapshots_allowed(enabled);
        }
    );
    connect(
        m_command, &CommandRow::set_inference_boxes,
        m_camera, [=](bool enabled){
            m_camera->set_overlay_enabled(enabled);
        }
    );

    m_instance_id = ProgramTracker::instance().add_console(program_id, *this);
}
SwitchSystem::~SwitchSystem(){
    ProgramTracker::instance().remove_console(m_instance_id);
    m_serial->stop();
}
ProgramState SwitchSystem::last_known_state() const{
    return m_command->last_known_state();
}
bool SwitchSystem::serial_ok() const{
    return m_serial->is_ready();
}
void SwitchSystem::wait_for_all_requests(){
    BotBase* botbase = this->botbase();
    if (botbase == nullptr){
        return;
    }
    botbase->wait_for_all_requests();
}
BotBase* SwitchSystem::botbase(){
    return m_serial->botbase().botbase();
}
VideoFeed& SwitchSystem::camera(){
    return *m_camera;
}
VideoOverlay& SwitchSystem::overlay(){
    return *m_camera;
}
void SwitchSystem::stop_serial(){
    m_serial->stop();
}
void SwitchSystem::reset_serial(){
    m_serial->reset();
}

VideoFeed& SwitchSystem::video(){
    return *m_camera;
}
BotBaseHandle& SwitchSystem::sender(){
    return m_serial->botbase();
}

void SwitchSystem::update_ui(ProgramState state){
    m_serial->botbase().set_allow_user_commands(state == ProgramState::STOPPED);
    switch (state){
    case ProgramState::NOT_READY:
        m_serial->set_options_enabled(false);
        m_camera->set_camera_enabled(false);
        m_camera->set_resolution_enabled(false);
        break;
    case ProgramState::STOPPED:
        m_serial->set_options_enabled(true);
        m_camera->set_camera_enabled(true);
        m_camera->set_resolution_enabled(true);
        break;
    case ProgramState::RUNNING:
//    case ProgramState::FINISHED:
    case ProgramState::STOPPING:
        m_serial->set_options_enabled(false);
#if 0
        if (m_factory.m_lock_camera_when_running){
            m_camera->set_camera_enabled(false);
        }
        if (m_factory.m_lock_resolution_when_running){
            m_camera->set_resolution_enabled(false);
        }
#endif
        break;
    }
    m_command->on_state_changed(state);
}

void SwitchSystem::resizeEvent(QResizeEvent* event){
    m_camera->update_size();
}
void SwitchSystem::keyPressEvent(QKeyEvent* event){
    m_command->on_key_press((Qt::Key)event->key());
}
void SwitchSystem::keyReleaseEvent(QKeyEvent* event){
    m_command->on_key_release((Qt::Key)event->key());
}
void SwitchSystem::focusInEvent(QFocusEvent* event){
    m_command->set_focus(true);
}

void SwitchSystem::focusOutEvent(QFocusEvent* event){
    m_command->set_focus(false);
}



}
}

