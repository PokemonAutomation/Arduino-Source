/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/Options/ConfigWidget.h"
#include "NintendoSwitch_CommandRow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




CommandRow::~CommandRow(){}
CommandRow::CommandRow(
    QWidget& parent,
    SwitchSystemSession& session,
    ConsoleModelCell& console_type,
    bool allow_commands_while_running
)
    : CommandRowWidget(parent, session)
    , m_session(session)
{
    m_label->setText("<b>Console Type:</b>");

    ConfigWidget* console_type_box = ConfigWidget::make_from_option(console_type, this);
    m_layout->insertWidget(0, &console_type_box->widget());
}









}
}













