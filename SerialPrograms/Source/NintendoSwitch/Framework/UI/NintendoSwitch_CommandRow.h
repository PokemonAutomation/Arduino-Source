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
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "NintendoSwitch/Framework/NintendoSwitch_VirtualController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class CommandRow : public QWidget, public VirtualController, public VideoOverlaySession::Listener{
    Q_OBJECT

public:
    ~CommandRow();
    CommandRow(
        QWidget& parent,
        BotBaseHandle& botbase,
        VideoOverlaySession& session,
        bool allow_commands_while_running
    );

    //  Returns false if key is not handled. (pass it up to next handler)
    bool on_key_press(Qt::Key key);
    bool on_key_release(Qt::Key key);

signals:
    void load_profile();
    void save_profile();
    void screenshot_requested();
    void video_requested();

public:
    void set_focus(bool focused);
    void update_ui();
    void on_state_changed(ProgramState state);

private:
    virtual void enabled_boxes(bool enabled) override;
    virtual void enabled_text (bool enabled) override;
    virtual void enabled_log  (bool enabled) override;
    virtual void enabled_stats(bool enabled) override;

private:
    BotBaseHandle& m_botbase;
    VideoOverlaySession& m_session;
    bool m_allow_commands_while_running;
    QComboBox* m_command_box;
    QLabel* m_status;

    QCheckBox* m_overlay_log;
    QCheckBox* m_overlay_text;
    QCheckBox* m_overlay_boxes;
    QCheckBox* m_overlay_stats;

    QPushButton* m_load_profile_button;
    QPushButton* m_save_profile_button;
    QPushButton* m_screenshot_button;
    QPushButton* m_video_button;
    bool m_last_known_focus;
};


}
}
#endif
