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
    bool allow_commands_while_running
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
    command_row->addWidget(m_status, 12);
    command_row->addSpacing(5);

    m_log_overlay_box = new QCheckBox("Log Overlay", this);
    m_log_overlay_box->setChecked(false);
    command_row->addWidget(m_log_overlay_box, 4);
    command_row->addSpacing(5);

    m_inference_box = new QCheckBox("Inference Overlay", this);
    m_inference_box->setChecked(true);
    command_row->addWidget(m_inference_box, 4);
    command_row->addSpacing(5);

    m_load_profile_button = new QPushButton("Load Profile", this);
    command_row->addWidget(m_load_profile_button, 2);

    m_save_profile_button = new QPushButton("Save Profile", this);
    command_row->addWidget(m_save_profile_button, 2);

    m_screenshot_button = new QPushButton("Screenshot", this);
    command_row->addWidget(m_screenshot_button, 2);


//    m_test_button = new QPushButton("Test Button", this);
//    command_row->addWidget(m_test_button, 3);

    update_ui();

    connect(
        m_log_overlay_box, &QCheckBox::stateChanged,
        this, [this](int){ emit set_log_text_overlay(m_log_overlay_box->isChecked()); }
    );
    connect(
        m_inference_box, &QCheckBox::stateChanged,
        this, [this](int){ emit set_inference_overlay(m_inference_box->isChecked()); }
    );
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
}

bool CommandRow::on_key_press(Qt::Key key){
    if (m_last_known_focus){
        return VirtualController::on_key_press(key);
    }
    return false;
}
bool CommandRow::on_key_release(Qt::Key key){
    if (m_last_known_focus){
        return VirtualController::on_key_release(key);
    }
    return false;
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
    bool stopped = last_known_state() == ProgramState::STOPPED;
    m_load_profile_button->setEnabled(stopped);
    if (!m_allow_commands_while_running){
//        m_reset_button->setEnabled(stopped);
        if (!stopped){
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Not Active. Keyboard commands are disabled while a program is running.", COLOR_PURPLE)
                )
            );
            return;
        }
    }

    BotBaseHandle::State state = m_botbase.state();
    if (state == BotBaseHandle::State::READY){
        if (!m_botbase.accepting_commands()){
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Not Active. The program on the device doesn't accept commands.", COLOR_RED)
                )
            );
        }else if (!m_last_known_focus){
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Not Active. Click on the video to activate keyboard commands.", COLOR_PURPLE)
                )
            );
        }else{
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Keyboard Control Active! Use the keyboard to enter commands.", COLOR_DARKGREEN)
                )
            );
        }
    }else{
        m_status->setText(
            QString::fromStdString(
                html_color_text("Not Active. The connection is not ready.", COLOR_RED)
            )
        );
    }
}

void CommandRow::on_state_changed(ProgramState state){
    VirtualController::on_state_changed(state);
    update_ui();
}




}
}












