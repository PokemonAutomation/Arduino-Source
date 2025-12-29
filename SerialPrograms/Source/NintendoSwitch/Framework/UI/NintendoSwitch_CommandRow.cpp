/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Panels/ConsoleSettingsStretch.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "ControllerInput/ControllerInput.h"
#include "ControllerInput/Keyboard/GlobalKeyboardHidTracker.h"
#include "NintendoSwitch_CommandRow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




CommandRow::~CommandRow(){
    global_input_remove_listener(*this);
    m_controller.remove_listener(*this);
    m_session.remove_listener(*this);
}
CommandRow::CommandRow(
    QWidget& parent,
    ControllerSession& controller,
    VideoOverlaySession& session,
    ConsoleModelCell& console_type,
    bool allow_commands_while_running
)
    : QWidget(&parent)
    , m_controller(controller)
    , m_session(session)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_last_known_focus(false)
    , m_last_known_state(ProgramState::STOPPED)
{
    QHBoxLayout* layout0 = new QHBoxLayout(this);
    layout0->setContentsMargins(0, 0, 0, 0);

    layout0->addWidget(new QLabel("<b>Console Type:</b>", this), CONSOLE_SETTINGS_STRETCH_L0_LABEL);

    QHBoxLayout* layout1 = new QHBoxLayout();
    layout0->addLayout(layout1, CONSOLE_SETTINGS_STRETCH_L0_RIGHT);
    layout1->setContentsMargins(0, 0, 0, 0);

    ConfigWidget* console_type_box = ConfigWidget::make_from_option(console_type, this);
    layout1->addWidget(&console_type_box->widget());



    layout1->addStretch(100);

    m_status = new QLabel(this);
//    m_status->setVisible(false);
    layout1->addWidget(m_status);
    layout1->addSpacing(5);

//    row->addWidget(new QLabel("<b>Overlays:<b>", this));

    CheckboxDropdown* overlays = new CheckboxDropdown(this, "Overlays");
    overlays->setMinimumWidth(80);
    {
        m_overlay_stats = overlays->addItem("Stats");
        m_overlay_stats->setChecked(session.enabled_stats());
    }
    {
        m_overlay_boxes = overlays->addItem("Boxes");
        m_overlay_boxes->setChecked(session.enabled_boxes());
    }
    if (PreloadSettings::instance().DEVELOPER_MODE){
        m_overlay_text = overlays->addItem("Text");  //  Nothing uses text overlay yet.
        m_overlay_text->setChecked(session.enabled_text());
    }
    if (PreloadSettings::instance().DEVELOPER_MODE){
        m_overlay_images = overlays->addItem("Masks");
        m_overlay_images->setChecked(session.enabled_images());
    }
    {
        m_overlay_log = overlays->addItem("Log");
        m_overlay_log->setChecked(session.enabled_log());
    }
    layout1->addWidget(overlays);

//    layout1->addSpacing(5);

    m_load_profile_button = new QPushButton("Load Profile", this);
    layout1->addWidget(m_load_profile_button, 2);

    m_save_profile_button = new QPushButton("Save Profile", this);
    layout1->addWidget(m_save_profile_button, 2);

    m_screenshot_button = new QPushButton("Screenshot", this);
//    m_screenshot_button->setToolTip("Take a screenshot of the console and save to disk.");
    layout1->addWidget(m_screenshot_button, 2);

    update_ui();

#if 1
    if (m_overlay_stats){
        connect(
            m_overlay_stats, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.set_enabled_stats(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_boxes){
        connect(
            m_overlay_boxes, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){

                m_session.set_enabled_boxes(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_text){
        connect(
            m_overlay_text, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.set_enabled_text(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_images){
        connect(
            m_overlay_images, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.set_enabled_images(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_log){
        connect(
            m_overlay_log, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.set_enabled_log(state == Qt::Checked);
            }
        );
    }
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
        layout1->addWidget(m_video_button, 2);
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
//    global_input_add_listener(*this);
}


bool CommandRow::allow_controller_input() const{
    return m_allow_commands_while_running || m_last_known_state == ProgramState::STOPPED;
}
void CommandRow::run_controller_input(ControllerInputState& state){
    if (!m_last_known_focus){
        m_controller.logger().log("Keyboard Command Suppressed: Not in focus.", COLOR_RED);
        return;
    }
    AbstractController* controller = m_controller.controller();
    if (controller == nullptr){
        m_controller.logger().log("Keyboard Command Suppressed: Controller is null.", COLOR_RED);
        return;
    }
    if (!allow_controller_input()){
        m_controller.logger().log("Keyboard Command Suppressed: Program is running.", COLOR_RED);
        return;
    }
    controller->run_controller_input(state);
}
void CommandRow::set_focus(bool focused){
    if (focused){
        global_input_add_listener(*this);
        if (allow_controller_input()){
        }
    }else{
        global_input_clear_state();
        global_input_remove_listener(*this);

        AbstractController* controller = m_controller.controller();
        if (controller != nullptr && allow_controller_input()){
            try{
                controller->cancel_all_commands();
            }catch (InvalidConnectionStateException&){
            }catch (OperationCancelledException&){
            }
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
                    "Keyboard: " + html_color_text("&#x2b24;", COLOR_PURPLE)
                )
            );
            return;
        }
    }


    if (!m_controller.ready()){
        m_status->setText(
            QString::fromStdString(
                "Keyboard: " + html_color_text("&#x2b24;", COLOR_RED)
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
                "Keyboard: " + html_color_text("&#x2b24;", COLOR_PURPLE)
            )
        );
        return;
    }

    m_status->setText(
        QString::fromStdString(
            "Keyboard: " + html_color_text("&#x2b24;", COLOR_DARKGREEN)
        )
    );
}

void CommandRow::on_state_changed(ProgramState state){
    m_last_known_state = state;
    if (allow_controller_input()){
        global_input_clear_state();
//        global_input_add_listener(*this);
    }else{
//        global_input_remove_listener(*this);
    }
    update_ui();
}


void CommandRow::on_overlay_enabled_stats(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_stats){
            m_overlay_stats->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_boxes(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_boxes){
            m_overlay_boxes->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_text(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_text){
            m_overlay_text->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_images(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_images){
            m_overlay_images->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRow::on_overlay_enabled_log(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_log){
            m_overlay_log->setChecked(enabled);
        }
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













