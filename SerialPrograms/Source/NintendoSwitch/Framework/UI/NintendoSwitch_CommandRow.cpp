/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "NintendoSwitch_CommandRow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


CommandRow::~CommandRow(){
    m_controller.remove_listener(*this);
    m_session.remove_listener(*this);
}
CommandRow::CommandRow(
    QWidget& parent,
    ControllerSession& controller,
    VideoOverlaySession& session,
    bool allow_commands_while_running
)
    : QWidget(&parent)
    , m_controller(controller)
    , m_session(session)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_last_known_focus(false)
    , m_last_known_state(ProgramState::STOPPED)
{
    QHBoxLayout* command_row = new QHBoxLayout(this);
    command_row->setContentsMargins(0, 0, 0, 0);

    command_row->addWidget(new QLabel("<b>Keyboard Input:</b>", this), 2);
    command_row->addSpacing(5);

    m_status = new QLabel(this);
    command_row->addWidget(m_status, 12);
    command_row->addSpacing(5);

    command_row->addWidget(new QLabel("<b>Overlays:<b>", this));

    m_overlay_boxes = new QCheckBox("Boxes", this);
    m_overlay_boxes->setChecked(session.enabled_boxes());
    command_row->addWidget(m_overlay_boxes);

    m_overlay_text = new QCheckBox("Text", this);
    m_overlay_text->setHidden(true);    //  Nothing uses text overlay yet.
    m_overlay_text->setChecked(session.enabled_text());
    command_row->addWidget(m_overlay_text);

    m_overlay_images = new QCheckBox("Images/Masks", this);
    m_overlay_images->setChecked(session.enabled_images());
    command_row->addWidget(m_overlay_images);


    m_overlay_log = new QCheckBox("Log", this);
    m_overlay_log->setChecked(session.enabled_log());
    command_row->addWidget(m_overlay_log);

    m_overlay_stats = new QCheckBox("Stats", this);
    m_overlay_stats->setChecked(session.enabled_stats());
    command_row->addWidget(m_overlay_stats);

    command_row->addSpacing(5);

    m_load_profile_button = new QPushButton("Load Profile", this);
    command_row->addWidget(m_load_profile_button, 2);

    m_save_profile_button = new QPushButton("Save Profile", this);
    command_row->addWidget(m_save_profile_button, 2);

    m_screenshot_button = new QPushButton("Screenshot", this);
//    m_screenshot_button->setToolTip("Take a screenshot of the console and save to disk.");
    command_row->addWidget(m_screenshot_button, 2);


//    m_test_button = new QPushButton("Test Button", this);
//    command_row->addWidget(m_test_button, 3);

    update_ui();

    connect(
        m_overlay_boxes, &QCheckBox::clicked,
        this, [this](bool checked){ m_session.set_enabled_boxes(checked); }
    );
#if QT_VERSION < 0x060700
    connect(
        m_overlay_text, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_text(checked); }
    );
    connect(
        m_overlay_images, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_images(checked); }
    );
    connect(
        m_overlay_log, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_log(checked); }
    );
    connect(
        m_overlay_stats, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_stats(checked); }
    );
#else
    connect(
        m_overlay_text, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState state){ m_session.set_enabled_text(state == Qt::Checked); }
    );
    connect(
        m_overlay_images, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState state){ m_session.set_enabled_images(state == Qt::Checked); }
    );
    connect(
        m_overlay_log, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState state){ m_session.set_enabled_log(state == Qt::Checked); }
    );
    connect(
        m_overlay_stats, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState state){ m_session.set_enabled_stats(state == Qt::Checked); }
    );
#endif
    connect(
        m_load_profile_button, &QPushButton::clicked,
        this, [this](bool) { emit load_profile(); }
    );
    connect(
        m_save_profile_button, &QPushButton::clicked,
        this, [this](bool) { emit save_profile(); }
    );
    connect(
        m_screenshot_button, &QPushButton::clicked,
        this, [this](bool){ emit screenshot_requested(); }
    );

#if (QT_VERSION_MAJOR == 6) && (QT_VERSION_MINOR >= 8)
    if (IS_BETA_VERSION || PreloadSettings::instance().DEVELOPER_MODE){
        m_video_button = new QPushButton("Video Capture", this);
        command_row->addWidget(m_video_button, 2);
        if (GlobalSettings::instance().STREAM_HISTORY->enabled()){
            connect(
                m_video_button, &QPushButton::clicked,
                this, [this](bool){ emit video_requested(); }
            );
        }else{
            m_video_button->setEnabled(false);
            m_video_button->setToolTip("Please turn on Stream History to enable video capture.");
        }
    }
#endif

    m_session.add_listener(*this);
    m_controller.add_listener(*this);
}

void CommandRow::on_key_press(const QKeyEvent& key){
    if (!m_last_known_focus){
        m_controller.logger().log("Keyboard Command Suppressed: Not in focus.", COLOR_RED);
        return;
    }
    AbstractController* controller = m_controller.controller();
    if (controller == nullptr){
        m_controller.logger().log("Keyboard Command Suppressed: Controller is null.", COLOR_RED);
        return;
    }
    if (!m_allow_commands_while_running && m_last_known_state != ProgramState::STOPPED){
        m_controller.logger().log("Keyboard Command Suppressed: Program is running.", COLOR_RED);
        return;
    }
    controller->keyboard_press(key);
}
void CommandRow::on_key_release(const QKeyEvent& key){
    if (!m_last_known_focus){
        return;
    }
    AbstractController* controller = m_controller.controller();
    if (controller == nullptr){
        return;
    }
    controller->keyboard_release(key);
}

void CommandRow::set_focus(bool focused){
    AbstractController* controller = m_controller.controller();
    if (!focused){
        if (controller != nullptr){
            controller->keyboard_release_all();
        }
    }
    if (m_last_known_focus == focused){
        return;
    }
    m_last_known_focus = focused;
    update_ui();
}

void CommandRow::update_ui(){
//    cout << "CommandRow::update_ui(): focus = " << m_last_known_focus << endl;

    bool stopped = m_last_known_state == ProgramState::STOPPED;
    m_load_profile_button->setEnabled(stopped);
    if (!m_allow_commands_while_running){
//        m_reset_button->setEnabled(stopped);
        if (!stopped){
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Not Active. A program is running.", COLOR_PURPLE)
                )
            );
            return;
        }
    }


    if (!m_controller.ready()){
        m_status->setText(
            QString::fromStdString(
                html_color_text("The controller is not ready.", COLOR_RED)
            )
        );
        return;
    }

    std::string error = m_controller.user_input_blocked();
    if (!error.empty()){
        m_status->setText(QString::fromStdString(error));
        return;
    }

    if (!m_last_known_focus){
        m_status->setText(
            QString::fromStdString(
                html_color_text("Click on the video to enable.", COLOR_PURPLE)
            )
        );
        return;
    }

    m_status->setText(
        QString::fromStdString(
            html_color_text("Keyboard Control Active!", COLOR_DARKGREEN)
        )
    );
}

void CommandRow::on_state_changed(ProgramState state){
    m_last_known_state = state;
    if (m_allow_commands_while_running || state == ProgramState::STOPPED){
        AbstractController* controller = m_controller.controller();
        if (controller != nullptr){
            controller->keyboard_release_all();
        }
    }
    update_ui();
}


void CommandRow::on_overlay_enabled_boxes(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_boxes->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_text(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_text->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_images(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_images->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_log(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_log->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_stats(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_stats->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void CommandRow::ready_changed(bool ready){
//    cout << "CommandRow::ready_changed(): " << ready << endl;
    QMetaObject::invokeMethod(this, [this]{
        update_ui();
    }, Qt::QueuedConnection);
}




}
}












