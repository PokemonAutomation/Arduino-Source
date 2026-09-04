/*  Command Row (Qt Widget)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_CommandRowWidget_H
#define PokemonAutomation_ConsoleInfra_CommandRowWidget_H

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "Common/Qt/CheckboxDropdown.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySession.h"
#include "ConsoleSystemSession.h"

namespace PokemonAutomation{
namespace ConsoleInfra{


class CommandRowWidget
    : public QWidget
    , public ConsoleSystemSession::Listener
    , public VideoOverlaySession::ContentListener
{
public:
    ~CommandRowWidget();
    CommandRowWidget(
        QWidget& parent,
        ConsoleSystemSession& session
    );


private:
    virtual void on_input_status_change(const std::string& status) override;
    virtual void on_lock_controllers() override;
    virtual void on_unlock_controllers() override;

    virtual void on_overlay_enabled_stats  (bool enabled) override;
    virtual void on_overlay_enabled_boxes  (bool enabled) override;
    virtual void on_overlay_enabled_text   (bool enabled) override;
    virtual void on_overlay_enabled_images (bool enabled) override;
    virtual void on_overlay_enabled_log    (bool enabled) override;


protected:
    ConsoleSystemSession& m_session;

    QLabel* m_label = nullptr;
    QHBoxLayout* m_layout = nullptr;

    QLabel* m_status = nullptr;

    CheckboxDropdownItem* m_overlay_stats = nullptr;
    CheckboxDropdownItem* m_overlay_boxes = nullptr;
    CheckboxDropdownItem* m_overlay_text = nullptr;
    CheckboxDropdownItem* m_overlay_images = nullptr;
    CheckboxDropdownItem* m_overlay_log = nullptr;

    QPushButton* m_load_profile_button = nullptr;
    QPushButton* m_save_profile_button = nullptr;
    QPushButton* m_screenshot_button = nullptr;
    QPushButton* m_video_button = nullptr;
};



}
}
#endif
