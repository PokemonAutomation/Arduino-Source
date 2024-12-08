/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/FireForgetDispatcher.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/AudioPipeline/UI/AudioSelectorWidget.h"
#include "CommonFramework/AudioPipeline/UI/AudioDisplayWidget.h"
#include "CommonFramework/ControllerDevices/SerialPortWidget.h"
#include "CommonFramework/VideoPipeline/UI/CameraSelectorWidget.h"
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "CommonFramework/Globals.h"
#include "NintendoSwitch_CommandRow.h"
#include "NintendoSwitch_SwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



SwitchSystemWidget::~SwitchSystemWidget(){
    m_serial_widget->stop();

    //  Delete all the UI elements first since they reference the states.
    delete m_audio_display;
    delete m_audio_widget;
    delete m_video_display;
    delete m_camera_widget;
    delete m_serial_widget;
}

SwitchSystemWidget::SwitchSystemWidget(
    QWidget& parent,
    SwitchSystemSession& session,
    uint64_t program_id
)
    : QWidget(&parent)
    , m_session(session)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);

    m_group_box = new CollapsibleGroupBox(*this, "Console " + QString::number(m_session.console_number()) + " Settings");
    layout->addWidget(m_group_box);

    QWidget* widget = new QWidget(m_group_box);
    m_group_box->set_widget(widget);
    {
        QVBoxLayout* group_layout = new QVBoxLayout(widget);
        group_layout->setAlignment(Qt::AlignTop);
        group_layout->setContentsMargins(0, 0, 0, 0);

        m_serial_widget = new SerialPortWidget(*this, m_session.serial_session(), m_session.logger());
        group_layout->addWidget(m_serial_widget);

        m_audio_display = new AudioDisplayWidget(*this, m_session.logger(), m_session.audio_session());
        layout->addWidget(m_audio_display);

        QVBoxLayout* video_holder = new QVBoxLayout();
        layout->addLayout(video_holder);
        video_holder->setContentsMargins(0, 0, 0, 0);
        m_video_display = new VideoDisplayWidget(
            *this, *video_holder,
            m_session.console_number(),
            *this,
            m_session.camera_session(),
            m_session.overlay_session()
        );
        video_holder->addWidget(m_video_display);

        m_camera_widget = new CameraSelectorWidget(
            m_session.camera_session(),
            m_session.logger(),
            *m_video_display
        );
        group_layout->addWidget(m_camera_widget);

        m_audio_widget = new AudioSelectorWidget(*widget, m_session.logger(), m_session.audio_session());
        group_layout->addWidget(m_audio_widget);

        m_command = new CommandRow(
            *widget,
            m_serial_widget->botbase(),
            m_session.overlay_session(),
            m_session.allow_commands_while_running()
        );
        group_layout->addWidget(m_command);
    }

    setFocusPolicy(Qt::StrongFocus);


    connect(
        m_serial_widget, &SerialPortWidget::signal_on_ready,
        m_command, [this](bool ready){
            m_command->update_ui();
        }
    );
    connect(
        m_command, &CommandRow::load_profile,
        m_command, [this](){
            std::string path = QFileDialog::getOpenFileName(this, tr("Choose the name of your profile file"), "", tr("JSON files (*.json)")).toStdString();
            if (path.empty()){
                return;
            }

            SwitchSystemOption option(m_session.min_pabotbase(), m_session.allow_commands_while_running());

            //  Deserialize into this local option instance.
            option.load_json(load_json_file(path));

            m_session.set(option);
        }
    );
    connect(
        m_command, &CommandRow::save_profile,
        m_command, [this](){
            std::string path = QFileDialog::getSaveFileName(this, tr("Choose the name of your profile file"), "", tr("JSON files (*.json)")).toStdString();
            if (path.empty()){
                return;
            }

            // Create a copy of option, to be able to serialize it later on
            SwitchSystemOption option(m_session.min_pabotbase(), m_session.allow_commands_while_running());

            m_session.get(option);

            option.to_json().dump(path);
        }
    );
    connect(
        m_command, &CommandRow::screenshot_requested,
        m_video_display, [this](){
            global_dispatcher.dispatch([this]{
                VideoSnapshot image = m_session.camera_session().snapshot();
                if (!image){
                    return;
                }
                std::string filename = SCREENSHOTS_PATH() + "screenshot-" + now_to_filestring() + ".png";
                m_session.logger().log("Saving screenshot to: " + filename, COLOR_PURPLE);
                image->save(filename);
            });
        }
    );
    connect(
        m_command, &CommandRow::video_requested,
        m_video_display, [this](){
            global_dispatcher.dispatch([this]{
                std::string filename = SCREENSHOTS_PATH() + "video-" + now_to_filestring() + ".mp4";
                m_session.logger().log("Saving screenshot to: " + filename, COLOR_PURPLE);
                m_session.save_history(filename);
            });
        }
    );
}


void SwitchSystemWidget::update_ui(ProgramState state){
    if (!m_session.allow_commands_while_running()){
        m_session.set_allow_user_commands(state == ProgramState::STOPPED);
    }
    switch (state){
    case ProgramState::NOT_READY:
        m_serial_widget->set_options_enabled(false);
        break;
    case ProgramState::STOPPED:
        m_serial_widget->set_options_enabled(true);
        break;
    case ProgramState::RUNNING:
    case ProgramState::STOPPING:
        m_serial_widget->set_options_enabled(false);
        break;
    }
    m_command->on_state_changed(state);
}

bool SwitchSystemWidget::key_press(QKeyEvent* event){
    return m_command->on_key_press((Qt::Key)event->key());
}

bool SwitchSystemWidget::key_release(QKeyEvent* event){
    return m_command->on_key_release((Qt::Key)event->key());
}

void SwitchSystemWidget::focus_in(QFocusEvent* event){
    m_command->set_focus(true);
}

void SwitchSystemWidget::focus_out(QFocusEvent* event){
    m_command->set_focus(false);
}

void SwitchSystemWidget::keyPressEvent(QKeyEvent* event){
    if (!key_press(event)){
        QWidget::keyPressEvent(event);
    }
}
void SwitchSystemWidget::keyReleaseEvent(QKeyEvent* event){
    if (!key_release(event)){
        QWidget::keyReleaseEvent(event);
    }
}
void SwitchSystemWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent" << endl;
    focus_in(event);
    QWidget::focusInEvent(event);
}
void SwitchSystemWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent" << endl;
    focus_out(event);
    QWidget::focusOutEvent(event);
}



}
}
