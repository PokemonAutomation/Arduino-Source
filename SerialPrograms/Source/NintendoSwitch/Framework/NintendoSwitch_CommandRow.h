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
#include "NintendoSwitch_VirtualController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class CommandRow : public QWidget, public VirtualController{
    Q_OBJECT

public:
    CommandRow(
        QWidget& parent,
        BotBaseHandle& botbase,
        LoggerQt& logger,
        FeedbackType feedback, bool allow_commands_while_running
    );

    void on_key_press(Qt::Key key);
    void on_key_release(Qt::Key key);

signals:
    void set_feedback_enabled(bool enabled);
    void set_inference_boxes(bool enabled);
    void screenshot_requested();

public slots:
    void set_focus(bool focused);
    void update_ui();
    void on_state_changed(ProgramState state);

private:
    BotBaseHandle& m_botbase;
    bool m_allow_commands_while_running;
    QComboBox* m_command_box;
    QLabel* m_status;
    QCheckBox* m_inference_box;
    QCheckBox* m_overlay_box;
    QPushButton* m_screenshot_button;
    bool m_last_known_focus;
};


}
}
#endif
