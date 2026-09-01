/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QFileDialog>
#include "Common/Cpp/ColoredText.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "NintendoSwitch_CommandRow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




CommandRow::~CommandRow(){
    m_session.controller().remove_listener(*this);
    m_session.overlay().remove_hid_listener(*this);
}
CommandRow::CommandRow(
    QWidget& parent,
    SwitchSystemSession& session,
    ConsoleModelCell& console_type,
    bool allow_commands_while_running
)
    : CommandRowWidget(parent, session)
    , m_session(session)
    , m_allow_commands_while_running(allow_commands_while_running)
    , m_last_known_focus(false)
    , m_last_known_state(ProgramState::STOPPED)
{
    m_label->setText("<b>Console Type:</b>");

    ConfigWidget* console_type_box = ConfigWidget::make_from_option(console_type, this);
    m_layout->insertWidget(0, &console_type_box->widget());




    m_status = new QLabel(this);
//    m_status->setVisible(false);
    m_layout->insertWidget(2, m_status);
    m_layout->addSpacing(5);

//    row->addWidget(new QLabel("<b>Overlays:<b>", this));

//    layout1->addSpacing(5);

    update_ui();

    m_session.overlay().add_hid_listener(*this);
    m_session.controller().add_listener(*this);
}



void CommandRow::on_focus_in(){
    if (m_last_known_focus){
        return;
    }
    m_last_known_focus = true;
    update_ui();
}
void CommandRow::on_focus_out(){
    if (!m_last_known_focus){
        return;
    }
    m_last_known_focus = false;
    update_ui();
}

void CommandRow::update_ui(){
//    cout << "CommandRow::update_ui(): focus = " << m_last_known_focus << endl;

    bool stopped = m_last_known_state == ProgramState::STOPPED;
//    m_load_profile_button->setEnabled(stopped);
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


    if (!m_session.controller().ready()){
        m_status->setText(
            QString::fromStdString(
                "Keyboard: " + html_color_text("&#x2b24;", COLOR_RED)
            )
        );
        return;
    }

    std::string error = m_session.controller().user_input_blocked();
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
    update_ui();
}


void CommandRow::ready_changed(bool ready){
//    cout << "CommandRow::ready_changed(): " << ready << endl;
    QMetaObject::invokeMethod(this, [this]{
        update_ui();
    }, Qt::QueuedConnection);
}




}
}













