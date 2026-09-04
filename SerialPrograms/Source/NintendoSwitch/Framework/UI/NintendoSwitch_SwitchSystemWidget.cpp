/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
{
    m_group_layout->addWidget(
        new CommandRow(*m_group_box->widget(), session)
    );
}







}
}
