/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include "CommonFramework/GlobalSettingsPanel.h"
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

    command_row->addWidget(new QLabel("<b>Overlays:<b>", this));

    m_overlay_boxes = new QCheckBox("Boxes", this);
    m_overlay_boxes->setChecked(true);
    command_row->addWidget(m_overlay_boxes);

    m_overlay_text = new QCheckBox("Text", this);
    m_overlay_text->setChecked(true);
    command_row->addWidget(m_overlay_text);

    m_overlay_log = new QCheckBox("Log", this);
    m_overlay_log->setChecked(true);
    command_row->addWidget(m_overlay_log);

    m_overlay_stats = new QCheckBox("Stats", this);
    m_overlay_stats->setChecked(false);
    m_overlay_stats->setEnabled(PreloadSettings::instance().DEVELOPER_MODE);
    command_row->addWidget(m_overlay_stats);

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
        m_overlay_boxes, &QCheckBox::stateChanged,
        this, [this](int){ emit set_overlay_boxes(m_overlay_boxes->isChecked()); }
    );
    connect(
        m_overlay_text, &QCheckBox::stateChanged,
        this, [this](int){ emit set_overlay_text(m_overlay_text->isChecked()); }
    );
    connect(
        m_overlay_log, &QCheckBox::stateChanged,
        this, [this](int){ emit set_overlay_log(m_overlay_log->isChecked()); }
    );
    connect(
        m_overlay_stats, &QCheckBox::stateChanged,
        this, [this](int){ emit set_overlay_stats(m_overlay_stats->isChecked()); }
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
    if (!focused){
        clear_state();
    }
    if (m_last_known_focus == focused){
        return;
    }
    m_last_known_focus = focused;
    update_ui();
}

void CommandRow::update_ui(){
    bool stopped = last_known_state() == ProgramState::STOPPED;
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

    BotBaseHandle::State state = m_botbase.state();
    if (state == BotBaseHandle::State::READY){
        if (!m_botbase.accepting_commands()){
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Device does not accept commands.", COLOR_RED)
                )
            );
        }else if (!m_last_known_focus){
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Click on the video to enable.", COLOR_PURPLE)
                )
            );
        }else{
            m_status->setText(
                QString::fromStdString(
                    html_color_text("Keyboard Control Active!", COLOR_DARKGREEN)
                )
            );
        }
    }else{
        m_status->setText(
            QString::fromStdString(
                html_color_text("The connection is not ready.", COLOR_RED)
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












