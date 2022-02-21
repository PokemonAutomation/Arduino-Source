/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QGroupBox>
#include "Common/Compiler.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/FireForgetDispatcher.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/AudioDisplayWidget.h"
#include "CommonFramework/AudioPipeline/AudioSelectorWidget.h"
#include "CommonFramework/ControllerDevices/SerialSelectorWidget.h"
#include "CommonFramework/VideoPipeline/CameraSelectorWidget.h"
#include "CommonFramework/VideoPipeline/VideoDisplayWidget.h"
#include "NintendoSwitch_CommandRow.h"
#include "NintendoSwitch_SwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SwitchSystemWidget::SwitchSystemWidget(
    QWidget& parent,
    SwitchSystemFactory& factory,
    Logger& logger,
    uint64_t program_id
)
    : SwitchSetupWidget(parent, factory)
    , m_factory(factory)
    , m_logger(logger, factory.m_logger_tag)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);

    m_group_box = new CollapsibleGroupBox(*this, "Console " + QString::number(factory.m_console_id) + " Settings");
    layout->addWidget(m_group_box);

    QWidget* widget = new QWidget(m_group_box);
    m_group_box->set_widget(widget);
    QVBoxLayout* group_layout = new QVBoxLayout(widget);
    group_layout->setAlignment(Qt::AlignTop);
    group_layout->setContentsMargins(0, 0, 0, 0);

    {
        m_serial = factory.m_serial.make_ui(*widget, logger);
        group_layout->addWidget(m_serial);

        m_video_display = new VideoDisplayWidget(*this);
        m_audio_display = new AudioDisplayWidget(*this);

        m_camera = factory.m_camera.make_ui(*widget, logger, *m_video_display);
        group_layout->addWidget(m_camera);

        m_audio = factory.m_audio.make_ui(*widget, logger, *m_audio_display);
        group_layout->addWidget(m_audio);

        m_command = new CommandRow(
            *widget,
            m_serial->botbase(),
            m_logger,
            factory.m_feedback, factory.m_allow_commands_while_running
        );
        group_layout->addWidget(m_command);
    }

    layout->addWidget(m_audio_display);
    layout->addWidget(m_video_display);
    m_camera->reset_video();
    m_audio->reset_audio();

    setFocusPolicy(Qt::StrongFocus);


    connect(
        m_serial, &SerialSelectorWidget::on_ready,
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
    connect(
        m_command, &CommandRow::screenshot_requested,
        m_video_display, [=](){
            global_dispatcher.dispatch([=]{
                QImage image = m_video_display->snapshot();
                if (image.isNull()){
                    return;
                }
                QString filename = QString::fromStdString("screenshot-" + now_to_filestring() + ".png");
                m_logger.log("Saving screenshot to: " + filename, COLOR_PURPLE);
                image.save(filename);
            });
        }
    );

    m_instance_id = ProgramTracker::instance().add_console(program_id, *this);
}
SwitchSystemWidget::~SwitchSystemWidget(){
    ProgramTracker::instance().remove_console(m_instance_id);
    m_serial->stop();
}
ProgramState SwitchSystemWidget::last_known_state() const{
    return m_command->last_known_state();
}
bool SwitchSystemWidget::serial_ok() const{
    return m_serial->is_ready();
}
void SwitchSystemWidget::wait_for_all_requests(){
    BotBase* botbase = this->botbase();
    if (botbase == nullptr){
        return;
    }
    botbase->wait_for_all_requests();
}
BotBase* SwitchSystemWidget::botbase(){
    return m_serial->botbase().botbase();
}
VideoFeed& SwitchSystemWidget::camera(){
    return *m_camera;
}
VideoOverlay& SwitchSystemWidget::overlay(){
    return *m_video_display;
}
AudioFeed& SwitchSystemWidget::audio(){
    return *m_audio;
}
void SwitchSystemWidget::stop_serial(){
    m_serial->stop();
}
void SwitchSystemWidget::reset_serial(){
    m_serial->reset();
}

VideoFeed& SwitchSystemWidget::video(){
    return *m_camera;
}
BotBaseHandle& SwitchSystemWidget::sender(){
    return m_serial->botbase();
}

void SwitchSystemWidget::update_ui(ProgramState state){
    if (!m_factory.m_allow_commands_while_running){
        m_serial->botbase().set_allow_user_commands(state == ProgramState::STOPPED);
    }
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

void SwitchSystemWidget::keyPressEvent(QKeyEvent* event){
    m_command->on_key_press((Qt::Key)event->key());
}
void SwitchSystemWidget::keyReleaseEvent(QKeyEvent* event){
    m_command->on_key_release((Qt::Key)event->key());
}
void SwitchSystemWidget::focusInEvent(QFocusEvent* event){
    m_command->set_focus(true);
}
void SwitchSystemWidget::focusOutEvent(QFocusEvent* event){
    m_command->set_focus(false);
}



}
}
