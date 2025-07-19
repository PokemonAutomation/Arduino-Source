/*  Command Row
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "ML_ImageAnnotationCommandRow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ML{


ImageAnnotationCommandRow::~ImageAnnotationCommandRow(){
    m_session.remove_listener(*this);
}
ImageAnnotationCommandRow::ImageAnnotationCommandRow(
    QWidget& parent,
    VideoOverlaySession& session
)
    : QWidget(&parent)
    , m_session(session)
    , m_last_known_focus(false)
    , m_last_known_state(ProgramState::STOPPED)
{
    QHBoxLayout* command_row = new QHBoxLayout(this);
    command_row->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* row = new QHBoxLayout();
    command_row->addLayout(row, 12);


#if 0
    row->addWidget(new QLabel("<b>Keyboard Input:</b>", this), 2);
    row->addSpacing(5);
#endif

    row->addStretch(100);

    m_status = new QLabel(this);
//    m_status->setVisible(false);
    row->addWidget(m_status);
    row->addSpacing(5);

//    row->addWidget(new QLabel("<b>Overlays:<b>", this));

    m_overlay_boxes = new QCheckBox("Boxes", this);
    m_overlay_boxes->setChecked(session.enabled_boxes());
    row->addWidget(m_overlay_boxes);

    m_overlay_text = new QCheckBox("Text", this);
    m_overlay_text->setHidden(true);    //  Nothing uses text overlay yet.
    m_overlay_text->setChecked(session.enabled_text());
    row->addWidget(m_overlay_text);

    if (PreloadSettings::instance().DEVELOPER_MODE){
        m_overlay_images = new QCheckBox("Masks", this);
        m_overlay_images->setChecked(session.enabled_images());
        row->addWidget(m_overlay_images);
    }

    m_overlay_log = new QCheckBox("Log", this);
    m_overlay_log->setChecked(session.enabled_log());
    row->addWidget(m_overlay_log);

    m_overlay_stats = new QCheckBox("Stats", this);
    m_overlay_stats->setChecked(session.enabled_stats());
    row->addWidget(m_overlay_stats);

    row->addSpacing(5);

//    m_test_button = new QPushButton("Test Button", this);
//    row->addWidget(m_test_button, 3);

    update_ui();

    connect(
        m_overlay_boxes, &QCheckBox::clicked,
        this, [this](bool checked){ m_session.set_enabled_boxes(checked); }
    );
#if QT_VERSION < 0x060700
    connect(
        m_overlay_text, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_text(checked); }
    );
    connect(
        m_overlay_images, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_images(checked); }
    );
    connect(
        m_overlay_log, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_log(checked); }
    );
    connect(
        m_overlay_stats, &QCheckBox::stateChanged,
        this, [this](bool checked){ m_session.set_enabled_stats(checked); }
    );
#else
    connect(
        m_overlay_text, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState state){ m_session.set_enabled_text(state == Qt::Checked); }
    );
    if (PreloadSettings::instance().DEVELOPER_MODE){
        connect(
            m_overlay_images, &QCheckBox::checkStateChanged,
            this, [this](Qt::CheckState state){ m_session.set_enabled_images(state == Qt::Checked); }
        );
    }
    connect(
        m_overlay_log, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState state){ m_session.set_enabled_log(state == Qt::Checked); }
    );
    connect(
        m_overlay_stats, &QCheckBox::checkStateChanged,
        this, [this](Qt::CheckState state){ m_session.set_enabled_stats(state == Qt::Checked); }
    );
#endif

    m_session.add_listener(*this);
}

void ImageAnnotationCommandRow::on_key_press(const QKeyEvent& key){
    if (!m_last_known_focus){
        m_session.add_log("Keyboard Command Suppressed: Not in focus.", COLOR_RED);
        return;
    }
}
void ImageAnnotationCommandRow::on_key_release(const QKeyEvent& key){}

void ImageAnnotationCommandRow::set_focus(bool focused){
    if (m_last_known_focus == focused){
        return;
    }
    m_last_known_focus = focused;
    update_ui();
}

void ImageAnnotationCommandRow::update_ui(){
    if (!m_last_known_focus){
        m_status->setText(
            QString::fromStdString(
                "Keyboard: " + html_color_text("&#x2b24;", COLOR_PURPLE)
            )
        );
        return;
    }

    m_status->setText(
        QString::fromStdString(
            "Keyboard: " + html_color_text("&#x2b24;", COLOR_DARKGREEN)
        )
    );
}

void ImageAnnotationCommandRow::on_state_changed(ProgramState state){
    m_last_known_state = state;
    update_ui();
}


void ImageAnnotationCommandRow::on_overlay_enabled_boxes(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_boxes->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void ImageAnnotationCommandRow::on_overlay_enabled_text(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_text->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void ImageAnnotationCommandRow::on_overlay_enabled_images(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_images->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void ImageAnnotationCommandRow::on_overlay_enabled_log(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_log->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void ImageAnnotationCommandRow::on_overlay_enabled_stats(bool enabled){
    QMetaObject::invokeMethod(this, [this, enabled]{
        this->m_overlay_stats->setChecked(enabled);
    }, Qt::QueuedConnection);
}
void ImageAnnotationCommandRow::ready_changed(bool ready){
//    cout << "ImageAnnotationCommandRow::ready_changed(): " << ready << endl;
    QMetaObject::invokeMethod(this, [this]{
        update_ui();
    }, Qt::QueuedConnection);
}




}
}












