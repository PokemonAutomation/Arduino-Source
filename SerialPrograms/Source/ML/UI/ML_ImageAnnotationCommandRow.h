/*  ML Image Annotation Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef POKEMON_AUTOMATION_ML_ImageAnnotationCommandRow_H
#define POKEMON_AUTOMATION_ML_ImageAnnotationCommandRow_H

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "Controllers/ControllerSession.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"

namespace PokemonAutomation{
namespace ML{


// UI that shows the checkerboxes to control whether to show video overlay elements.
// e.g. checkerbox to toggle on/off overlay boxes
class ImageAnnotationCommandRow :
    public QWidget,
    public VideoOverlaySession::ContentListener,
    public ControllerSession::Listener
{
    Q_OBJECT

public:
    ~ImageAnnotationCommandRow();
    ImageAnnotationCommandRow(
        QWidget& parent,
        VideoOverlaySession& session
    );

    void on_key_press(const QKeyEvent& key);
    void on_key_release(const QKeyEvent& key);

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
    VideoOverlaySession& m_session;
    QComboBox* m_command_box;
    QLabel* m_status;

    QCheckBox* m_overlay_log;
    QCheckBox* m_overlay_text;
    QCheckBox* m_overlay_images;
    QCheckBox* m_overlay_boxes;
    QCheckBox* m_overlay_stats;

    bool m_last_known_focus;
    ProgramState m_last_known_state;
};


}
}
#endif
