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
#include "Controllers/ControllerSession.h"
#include "ConsoleInfra/CommandRowWidget.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


// UI that shows the checkerboxes to control whether to show video overlay elements.
// e.g. checkerbox to toggle on/off overlay boxes
class CommandRow
    : public ConsoleInfra::CommandRowWidget
    , public VideoDisplayHidListener
    , public ControllerSession::Listener
{
public:
    ~CommandRow();
    CommandRow(
        QWidget& parent,
        SwitchSystemSession& session,
        ConsoleModelCell& console_type,
        bool allow_commands_while_running
    );

public:
    void update_ui();
    void on_state_changed(ProgramState state);

private:
    virtual void ready_changed(bool ready) override;

    virtual void on_focus_in() override;
    virtual void on_focus_out() override;

private:
    SwitchSystemSession& m_session;
    bool m_allow_commands_while_running;

    QLabel* m_status = nullptr;
    bool m_last_known_focus;
    ProgramState m_last_known_state;
};


}
}
#endif
