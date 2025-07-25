/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
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
#include "Controllers/ControllerSession.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


// UI that shows the checkerboxes to control whether to show video overlay elements.
// e.g. checkerbox to toggle on/off overlay boxes
class CommandRow :
    public QWidget,
    public VideoOverlaySession::ContentListener,
    public ControllerSession::Listener
{
    Q_OBJECT

public:
    ~CommandRow();
    CommandRow(
        QWidget& parent,
        ControllerSession& controller,
        VideoOverlaySession& session,
        ConsoleModelCell& console_type,
        bool allow_commands_while_running
    );

    void on_key_press(const QKeyEvent& key);
    void on_key_release(const QKeyEvent& key);

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
    virtual void on_overlay_enabled_boxes  (bool enabled) override;
    virtual void on_overlay_enabled_text   (bool enabled) override;
    virtual void on_overlay_enabled_images (bool enabled) override;
    virtual void on_overlay_enabled_log    (bool enabled) override;
    virtual void on_overlay_enabled_stats  (bool enabled) override;
    virtual void ready_changed(bool ready) override;

private:
    ControllerSession& m_controller;
    VideoOverlaySession& m_session;
    bool m_allow_commands_while_running;
    QComboBox* m_command_box = nullptr;
    QLabel* m_status = nullptr;

    QCheckBox* m_overlay_log = nullptr;
    QCheckBox* m_overlay_text = nullptr;
    QCheckBox* m_overlay_images = nullptr;
    QCheckBox* m_overlay_boxes = nullptr;
    QCheckBox* m_overlay_stats = nullptr;

    QPushButton* m_load_profile_button = nullptr;
    QPushButton* m_save_profile_button = nullptr;
    QPushButton* m_screenshot_button = nullptr;
    QPushButton* m_video_button = nullptr;
    bool m_last_known_focus;
    ProgramState m_last_known_state;
};


}
}
#endif
