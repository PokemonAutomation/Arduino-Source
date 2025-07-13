/*  ML Image Display Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/FireForgetDispatcher.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/UI/VideoSourceSelectorWidget.h"
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_CommandRow.h"
#include "ML_ImageDisplayWidget.h"
#include "ML_ImageDisplaySession.h"
#include "ML_ImageDisplayOption.h"
#include "ML_ImageDisplayCommandRow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ML{


ImageDisplayWidget::~ImageDisplayWidget(){
    //  Delete all the UI elements first since they reference the states.
    delete m_video_display;
    delete m_video_selector;
}

ImageDisplayWidget::ImageDisplayWidget(
    QWidget& parent,
    ImageDisplaySession& session,
    uint64_t program_id
)
    : QWidget(&parent)
    , m_session(session)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);

    m_group_box = new CollapsibleGroupBox(*this, "Image Settings");
    layout->addWidget(m_group_box);

    QWidget* widget = new QWidget(m_group_box);
    m_group_box->set_widget(widget);
    {
        QVBoxLayout* video_holder = new QVBoxLayout();
        layout->addLayout(video_holder);
        video_holder->setContentsMargins(0, 0, 0, 0);

        m_video_display = new VideoDisplayWidget(
            *this, *video_holder,
            0,  // dummy console ID
            *this,
            m_session.video_session(),
            m_session.overlay_session()
        );
        video_holder->addWidget(m_video_display);
    }
    {
        QVBoxLayout* group_layout = new QVBoxLayout(widget);
        group_layout->setAlignment(Qt::AlignTop);
        group_layout->setContentsMargins(0, 0, 0, 0);

        m_video_selector = new VideoSourceSelectorWidget(m_session.logger(), m_session.video_session());
        group_layout->addWidget(m_video_selector);

        m_command = new ImageDisplayCommandRow(
            *widget,
            m_session.overlay_session()
        );
        group_layout->addWidget(m_command);
    }

    setFocusPolicy(Qt::StrongFocus);

    connect(
        m_command, &ImageDisplayCommandRow::load_profile,
        m_command, [this](){
            std::string path = QFileDialog::getOpenFileName(this, tr("Choose the name of your profile file"), "", tr("JSON files (*.json)")).toStdString();
            if (path.empty()){
                return;
            }

            ImageDisplayOption option;
            //  Deserialize into this local option instance.
            option.load_json(load_json_file(path));

            m_session.set(option);
        }
    );
    connect(
        m_command, &ImageDisplayCommandRow::save_profile,
        m_command, [this](){
            std::string path = QFileDialog::getSaveFileName(this, tr("Choose the name of your profile file"), "", tr("JSON files (*.json)")).toStdString();
            if (path.empty()){
                return;
            }

            //  Create a copy of option, to be able to serialize it later on
            ImageDisplayOption option;
            m_session.get(option);
            option.to_json().dump(path);
        }
    );
    
    connect(
        m_command, &ImageDisplayCommandRow::video_requested,
        m_video_display, [this](){
            global_dispatcher.dispatch([this]{
                std::string filename = SCREENSHOTS_PATH() + "video-" + now_to_filestring() + ".mp4";
                m_session.logger().log("Saving screenshot to: " + filename, COLOR_PURPLE);
            });
        }
    );
}


void ImageDisplayWidget::update_ui(ProgramState state){
    m_command->on_state_changed(state);
}

void ImageDisplayWidget::key_press(QKeyEvent* event){
//    cout << "press:   " << event->nativeVirtualKey() << endl;
    m_command->on_key_press(*event);
}

void ImageDisplayWidget::key_release(QKeyEvent* event){
//    cout << "release: " << event->nativeVirtualKey() << endl;
    m_command->on_key_release(*event);
}

void ImageDisplayWidget::focus_in(QFocusEvent* event){
    m_command->set_focus(true);
}

void ImageDisplayWidget::focus_out(QFocusEvent* event){
    m_command->set_focus(false);
}

void ImageDisplayWidget::keyPressEvent(QKeyEvent* event){
    key_press(event);
}
void ImageDisplayWidget::keyReleaseEvent(QKeyEvent* event){
    key_release(event);
}
void ImageDisplayWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent" << endl;
    focus_in(event);
    QWidget::focusInEvent(event);
}
void ImageDisplayWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent" << endl;
    focus_out(event);
    QWidget::focusOutEvent(event);
}



}
}
