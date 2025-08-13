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
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.h"
#include "NintendoSwitch/Framework/UI/NintendoSwitch_CommandRow.h"
#include "ML_ImageAnnotationDisplayWidget.h"
#include "ML_ImageAnnotationDisplaySession.h"
#include "ML_ImageAnnotationDisplayOption.h"
#include "ML_ImageAnnotationCommandRow.h"
#include "ML_ImageAnnotationSourceSelectorWidget.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace ML{


ImageAnnotationDisplayWidget::~ImageAnnotationDisplayWidget(){
    //  Delete all the UI elements first since they reference the states.
    delete m_video_display;
    delete m_selector_widget;
}

ImageAnnotationDisplayWidget::ImageAnnotationDisplayWidget(
    QWidget& parent,
    ImageAnnotationDisplaySession& session,
    CommandReceiver* command_receiver
)
    : QWidget(&parent)
    , m_session(session)
    , m_command_receiver(command_receiver)
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
            0,  // display source ID. We only display one image for annotation. So 0.
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

        m_selector_widget = new ImageAnnotationSourceSelectorWidget(m_session);
        group_layout->addWidget(m_selector_widget);

        m_command = new ImageAnnotationCommandRow(
            *widget,
            m_session.overlay_session()
        );
        group_layout->addWidget(m_command);
    }

    setFocusPolicy(Qt::StrongFocus);
}

std::string ImageAnnotationDisplayWidget::image_folder_path() const{
    return m_selector_widget->image_folder_path();
}

void ImageAnnotationDisplayWidget::update_ui(ProgramState state){
    m_command->on_state_changed(state);
}

void ImageAnnotationDisplayWidget::key_press(QKeyEvent* event){
//    cout << "press:   " << event->nativeVirtualKey() << endl;
    m_command->on_key_press(*event);
    if (m_command_receiver){
        m_command_receiver->key_press(event);
    }
}

void ImageAnnotationDisplayWidget::key_release(QKeyEvent* event){
//    cout << "release: " << event->nativeVirtualKey() << endl;
    m_command->on_key_release(*event);
    if (m_command_receiver){
        m_command_receiver->key_release(event);
    }
}

void ImageAnnotationDisplayWidget::focus_in(QFocusEvent* event){
    m_command->set_focus(true);
    if (m_command_receiver){
        m_command_receiver->focus_in(event);
    }
}

void ImageAnnotationDisplayWidget::focus_out(QFocusEvent* event){
    m_command->set_focus(false);
    if (m_command_receiver){
        m_command_receiver->focus_out(event);
    }
}

void ImageAnnotationDisplayWidget::keyPressEvent(QKeyEvent* event){
    key_press(event);
}
void ImageAnnotationDisplayWidget::keyReleaseEvent(QKeyEvent* event){
    key_release(event);
}
void ImageAnnotationDisplayWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent" << endl;
    focus_in(event);
    QWidget::focusInEvent(event);
}
void ImageAnnotationDisplayWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent" << endl;
    focus_out(event);
    QWidget::focusOutEvent(event);
}



}
}
