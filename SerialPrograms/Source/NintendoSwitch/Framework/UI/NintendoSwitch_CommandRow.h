/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_CommandRow_H
#define PokemonAutomation_NintendoSwitch_CommandRow_H

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include "CommonFramework/Globals.h"
#include "ConsoleInfra/CommandRowWidget.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


// UI that shows the checkerboxes to control whether to show video overlay elements.
// e.g. checkerbox to toggle on/off overlay boxes
class CommandRow : public ConsoleInfra::CommandRowWidget
{
public:
    ~CommandRow();
    CommandRow(
        QWidget& parent,
        SwitchSystemSession& session,
        ConsoleModelCell& console_type,
        bool allow_commands_while_running
    );

private:
    SwitchSystemSession& m_session;
};


}
}
#endif
