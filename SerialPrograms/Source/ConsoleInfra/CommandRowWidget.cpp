/*  Command Row (Qt Widget)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFileDialog>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/FireForgetDispatcher.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "CommonFramework/Panels/ConsoleSettingsStretch.h"
#include "CommandRowWidget.h"

namespace PokemonAutomation{
namespace ConsoleInfra{



CommandRowWidget::~CommandRowWidget(){
    m_session.remove_listener(*this);
    m_session.overlay().remove_content_listener(*this);
}
CommandRowWidget::CommandRowWidget(
    QWidget& parent,
    ConsoleSystemSession& session
)
    : QWidget(&parent)
    , m_session(session)
{
    QHBoxLayout* layout0 = new QHBoxLayout(this);
    layout0->setContentsMargins(0, 0, 0, 0);

    m_label = new QLabel("<b>Options:</b>", this);
    layout0->addWidget(m_label, CONSOLE_SETTINGS_STRETCH_L0_LABEL);

    m_layout = new QHBoxLayout();
    layout0->addLayout(m_layout, CONSOLE_SETTINGS_STRETCH_L0_RIGHT);
    m_layout->setContentsMargins(0, 0, 0, 0);


    m_layout->addStretch(100);


    m_status = new QLabel(QString::fromStdString(session.status()), this);
//    m_status->setVisible(false);
    m_layout->addWidget(m_status);
    m_layout->addSpacing(5);


    CheckboxDropdown* overlays = new CheckboxDropdown(this, "Overlays");
    overlays->setMinimumWidth(80);
    {
        m_overlay_stats = overlays->addItem("Stats");
        m_overlay_stats->setChecked(session.overlay().enabled_stats());
    }
    {
        m_overlay_boxes = overlays->addItem("Boxes");
        m_overlay_boxes->setChecked(session.overlay().enabled_boxes());
    }
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        m_overlay_text = overlays->addItem("Text");  //  Nothing uses text overlay yet.
        m_overlay_text->setChecked(session.overlay().enabled_text());
    }
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        m_overlay_images = overlays->addItem("Masks");
        m_overlay_images->setChecked(session.overlay().enabled_images());
    }
    {
        m_overlay_log = overlays->addItem("Log");
        m_overlay_log->setChecked(session.overlay().enabled_log());
    }
    m_layout->addWidget(overlays);

    m_load_profile_button = new QPushButton("Load Profile", this);
    m_layout->addWidget(m_load_profile_button, 2);

    m_save_profile_button = new QPushButton("Save Profile", this);
    m_layout->addWidget(m_save_profile_button, 2);

    m_screenshot_button = new QPushButton("Screenshot", this);
//    m_screenshot_button->setToolTip("Take a screenshot of the console and save to disk.");
    m_layout->addWidget(m_screenshot_button, 2);

    m_video_button = new QPushButton("Video Capture", this);
    m_layout->addWidget(m_video_button, 2);


    if (m_overlay_stats){
        connect(
            m_overlay_stats, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.overlay().set_enabled_stats(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_boxes){
        connect(
            m_overlay_boxes, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){

                m_session.overlay().set_enabled_boxes(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_text){
        connect(
            m_overlay_text, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.overlay().set_enabled_text(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_images){
        connect(
            m_overlay_images, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.overlay().set_enabled_images(state == Qt::Checked);
            }
        );
    }
    if (m_overlay_log){
        connect(
            m_overlay_log, &CheckboxDropdownItem::checkStateChanged,
            this, [this](Qt::CheckState state){
                m_session.overlay().set_enabled_log(state == Qt::Checked);
            }
        );
    }

    connect(
        m_load_profile_button, &QPushButton::clicked,
        this, [this](bool){
            std::string path = QFileDialog::getOpenFileName(
                this,
                tr("Choose the name of your profile file"),
                "",
                tr("JSON files (*.json)")
            ).toStdString();
            if (path.empty()){
                return;
            }

            ConsoleSystemOption option(
                m_session.controllers(),
                m_session.allow_commands_while_locked()
            );

            //  Deserialize into this local option instance.
            option.load_json(load_json_file(path));

            m_session.load(option);
        }
    );
    connect(
        m_save_profile_button, &QPushButton::clicked,
        this, [this](bool){
            std::string path = QFileDialog::getSaveFileName(
                this,
                tr("Choose the name of your profile file"),
                "",
                tr("JSON files (*.json)")
            ).toStdString();
            if (path.empty()){
                return;
            }

            //  Create a copy of option, to be able to serialize it later on
            ConsoleSystemOption option(
                m_session.controllers(),
                m_session.allow_commands_while_locked()
            );

            m_session.save(option);

            option.to_json().dump(path);
        }
    );
    connect(
        m_screenshot_button, &QPushButton::clicked,
        this, [this](bool){
            global_dispatcher.dispatch([this]{
                VideoSnapshot image = m_session.video().snapshot();
                if (!image){
                    return;
                }
                std::string filename = SCREENSHOTS_PATH() + "screenshot-" + now_to_filestring() + ".png";
                m_session.logger().log("Saving screenshot to: " + filename, COLOR_PURPLE);
                image->save(filename);
            });
        }
    );
    if (GlobalSettings::instance().STREAM_HISTORY->enabled()){
        connect(
            m_video_button, &QPushButton::clicked,
            this, [this](bool){
                global_dispatcher.dispatch([this]{
                    std::string filename = SCREENSHOTS_PATH() + "video-" + now_to_filestring() + ".mp4";
                    m_session.logger().log("Saving screenshot to: " + filename, COLOR_PURPLE);
                    m_session.save_history(filename);
                });
            }
        );
    }else{
        m_video_button->setEnabled(false);
        m_video_button->setToolTip("Please turn on Stream History to enable video capture.");
    }

    m_session.overlay().add_content_listener(*this);
    m_session.add_listener(*this);
}



void CommandRowWidget::on_input_status_change(const std::string& status){
    QMetaObject::invokeMethod(this, [this, status]{
        m_status->setText(QString::fromStdString(status));
    }, Qt::QueuedConnection);
}
void CommandRowWidget::on_lock_controllers(){
    QMetaObject::invokeMethod(this, [this]{
        m_load_profile_button->setEnabled(false);
    }, Qt::QueuedConnection);
}
void CommandRowWidget::on_unlock_controllers(){
    QMetaObject::invokeMethod(this, [this]{
        m_load_profile_button->setEnabled(true);
    }, Qt::QueuedConnection);
}

void CommandRowWidget::on_overlay_enabled_stats(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_stats){
            m_overlay_stats->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRowWidget::on_overlay_enabled_boxes(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_boxes){
            m_overlay_boxes->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRowWidget::on_overlay_enabled_text(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_text){
            m_overlay_text->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRowWidget::on_overlay_enabled_images(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_images){
            m_overlay_images->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}
void CommandRowWidget::on_overlay_enabled_log(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        if (m_overlay_log){
            m_overlay_log->setChecked(enabled);
        }
    }, Qt::QueuedConnection);
}


























}
}
