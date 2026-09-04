/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Globals.h"
#include "NintendoSwitch_CommandRow.h"
#include "NintendoSwitch_SwitchSystemWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<NintendoSwitch::SwitchSystemWidget>;

namespace NintendoSwitch{




SwitchSystemWidget::SwitchSystemWidget(
    QWidget& parent,
    SwitchSystemSession& session
)
    : ConsoleSystemWidget(parent, session, false)
    , m_session(session)
{
    m_command = new CommandRow(
        *m_group_box->widget(),
        m_session,
        m_session.console_type(),
        m_session.allow_commands_while_locked()
    );
    m_group_layout->addWidget(m_command);
}


void SwitchSystemWidget::update_ui(ProgramState state){
    if (state != ProgramState::STOPPED){
        m_session.lock_controllers("Program is Running");
    }else{
        m_session.unlock_controllers();
    }
}





}
}
