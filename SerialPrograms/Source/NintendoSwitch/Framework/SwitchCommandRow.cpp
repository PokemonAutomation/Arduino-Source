/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include "SwitchCommandRow.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


CommandRow::CommandRow(
    QWidget& parent,
    BotBaseHandle& botbase,
    Logger& logger,
    FeedbackType feedback
)
    : QWidget(&parent)
    , VirtualController(botbase, logger)
    , m_botbase(botbase)
    , m_last_known_focus(false)
{
    QHBoxLayout* command_row = new QHBoxLayout(this);
    command_row->setMargin(0);

    command_row->addWidget(new QLabel("<b>Keyboard Input:</b>", this), 1);
    command_row->addSpacing(5);

    m_status = new QLabel(this);
    command_row->addWidget(m_status, 5);
    command_row->addSpacing(5);

    m_inference_box = new QCheckBox("Enable Feedback", this);
    m_inference_box->setChecked(true);
    command_row->addWidget(m_inference_box, 2);
    command_row->addSpacing(5);

    m_overlay_box = new QCheckBox("Show Inference Boxes", this);
    m_overlay_box->setChecked(true);
    command_row->addWidget(m_overlay_box, 2);

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
            set_feedback_enabled(m_inference_box->isChecked());
        }
    );
    connect(
        m_overlay_box, &QCheckBox::stateChanged,
        this, [=](int){ set_inference_boxes(m_overlay_box->isChecked()); }
    );
#if 0
    connect(
        m_test_button, &QPushButton::clicked,
        this, [=](bool){
            test_button();
        }
    );
#endif
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
    bool stopped = last_known_state() == ProgramState::STOPPED;
//    m_reset_button->setEnabled(stopped);
    if (!stopped){
        m_status->setText(
            "<font color=\"purple\">Not Active. Keyboard commands are disabled while a program is running.</font>"
        );
        return;
    }

    BotBaseHandle::State state = m_botbase.state();
    if (state == BotBaseHandle::State::READY){
        if (!m_botbase.accepting_commands()){
            m_status->setText(
                "<font color=\"red\">Not Active. The program on the device doesn't accept commands.</font>"
            );
        }else if (!m_last_known_focus){
            m_status->setText(
                "<font color=\"purple\">Not Active. Click on the video to activate keyboard commands.</font>"
            );
        }else{
            m_status->setText(
                "<font color=\"green\">Keyboard Control Active! Use the keyboard to enter commands.</font>"
            );
        }
    }else{
        m_status->setText(
            "<font color=\"red\">Not Active. The connection is not ready.</font>"
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












