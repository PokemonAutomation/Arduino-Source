/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_CommandRow_H
#define PokemonAutomation_NintendoSwitch_CommandRow_H

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include "CommonFramework/Globals.h"
#include "NintendoSwitch/Framework/NintendoSwitch_VirtualController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class CommandRow : public QWidget, public VirtualController{
    Q_OBJECT

public:
    CommandRow(
        QWidget& parent,
        BotBaseHandle& botbase,
        bool allow_commands_while_running
    );

    //  Returns false if key is not handled. (pass it up to next handler)
    bool on_key_press(Qt::Key key);
    bool on_key_release(Qt::Key key);

signals:
    void set_inference_overlay(bool enabled);
    void set_log_text_overlay(bool enabled);
    void load_profile();
    void save_profile();
    void screenshot_requested();

public:
    void set_focus(bool focused);
    void update_ui();
    void on_state_changed(ProgramState state);

private:
    BotBaseHandle& m_botbase;
    bool m_allow_commands_while_running;
    QComboBox* m_command_box;
    QLabel* m_status;
    QCheckBox* m_log_overlay_box;
    QCheckBox* m_inference_box;
    QPushButton* m_load_profile_button;
    QPushButton* m_save_profile_button;
    QPushButton* m_screenshot_button;
    bool m_last_known_focus;
};


}
}
#endif
