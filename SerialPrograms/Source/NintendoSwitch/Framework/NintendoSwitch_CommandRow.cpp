/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "NintendoSwitch_CommandRow.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


CommandRow::CommandRow(
    QWidget& parent,
    BotBaseHandle& botbase,
    FeedbackType feedback, bool allow_commands_while_running
)
    : QWidget(&parent)
    , VirtualController(botbase, allow_commands_while_running)
    , m_botbase(botbase)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_last_known_focus(false)
{
    QHBoxLayout* command_row = new QHBoxLayout(this);
    command_row->setContentsMargins(0, 0, 0, 0);

    command_row->addWidget(new QLabel("<b>Keyboard Input:</b>", this), 2);
    command_row->addSpacing(5);

    m_status = new QLabel(this);
    command_row->addWidget(m_status, 10);
    command_row->addSpacing(5);

    m_inference_box = new QCheckBox("Enable Feedback", this);
    m_inference_box->setChecked(true);
    command_row->addWidget(m_inference_box, 3);
    command_row->addSpacing(5);

    m_overlay_box = new QCheckBox("Inference Boxes", this);
    m_overlay_box->setChecked(true);
    command_row->addWidget(m_overlay_box, 3);

    m_screenshot_button = new QPushButton("Screenshot", this);
    command_row->addWidget(m_screenshot_button, 2);

    switch (feedback){
    case FeedbackType::NONE:
        m_inference_box->setChecked(false);
        m_inference_box->setEnabled(false);
        m_overlay_box->setChecked(false);
        m_overlay_box->setEnabled(false);
        break;
    case FeedbackType::OPTIONAL_:
        m_inference_box->setChecked(true);
        m_inference_box->setEnabled(true);
        m_overlay_box->setChecked(true);
        m_overlay_box->setEnabled(true);
        break;
    case FeedbackType::REQUIRED:
        m_inference_box->setChecked(true);
        m_inference_box->setEnabled(false);
        m_overlay_box->setChecked(true);
        m_overlay_box->setEnabled(true);
        break;
    }


//    m_test_button = new QPushButton("Test Button", this);
//    command_row->addWidget(m_test_button, 3);

    update_ui();

    connect(
        m_inference_box, &QCheckBox::stateChanged,
        this, [=](int){
//            m_overlay_box->setEnabled(m_inference_box->isChecked());
            emit set_feedback_enabled(m_inference_box->isChecked());
        }
    );
    connect(
        m_overlay_box, &QCheckBox::stateChanged,
        this, [=](int){ emit set_inference_boxes(m_overlay_box->isChecked()); }
    );
    connect(
        m_screenshot_button, &QPushButton::clicked,
        this, [=](bool){ emit screenshot_requested(); }
    );
}

void CommandRow::on_key_press(Qt::Key key){
    if (m_last_known_focus){
        VirtualController::on_key_press(key);
    }
}
void CommandRow::on_key_release(Qt::Key key){
    if (m_last_known_focus){
        VirtualController::on_key_release(key);
    }
}

void CommandRow::set_focus(bool focused){
    if (m_last_known_focus == focused){
        return;
    }
    m_last_known_focus = focused;
    update_ui();
    if (!focused){
        clear_state();
    }
}

void CommandRow::update_ui(){
    if (!m_allow_commands_while_running){
        bool stopped = last_known_state() == ProgramState::STOPPED;
//        m_reset_button->setEnabled(stopped);
        if (!stopped){
            m_status->setText(
                html_color_text("Not Active. Keyboard commands are disabled while a program is running.", COLOR_PURPLE)
            );
            return;
        }
    }

    BotBaseHandle::State state = m_botbase.state();
    if (state == BotBaseHandle::State::READY){
        if (!m_botbase.accepting_commands()){
            m_status->setText(
                html_color_text("Not Active. The program on the device doesn't accept commands.", COLOR_RED)
            );
        }else if (!m_last_known_focus){
            m_status->setText(
                html_color_text("Not Active. Click on the video to activate keyboard commands.", COLOR_PURPLE)
            );
        }else{
            m_status->setText(
                html_color_text("Keyboard Control Active! Use the keyboard to enter commands.", COLOR_DARKGREEN)
            );
        }
    }else{
        m_status->setText(
            html_color_text("Not Active. The connection is not ready.", COLOR_RED)
        );
    }
}

void CommandRow::on_state_changed(ProgramState state){
    if (state == last_known_state()){
        return;
    }
    VirtualController::on_state_changed(state);
    update_ui();
}




}
}












