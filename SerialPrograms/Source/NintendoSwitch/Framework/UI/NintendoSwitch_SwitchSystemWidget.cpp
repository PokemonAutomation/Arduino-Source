/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/AudioPipeline/UI/AudioSelectorWidget.h"
#include "CommonFramework/AudioPipeline/UI/AudioDisplayWidget.h"
#include "CommonFramework/VideoPipeline/UI/VideoSourceSelectorWidget.h"
#include "CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h"
#include "Controllers/ControllerSelectorWidget.h"
#include "NintendoSwitch_CommandRow.h"
#include "NintendoSwitch_SwitchSystemWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SwitchSystemWidget::~SwitchSystemWidget(){
    //  Delete all the UI elements first since they reference the states.
    delete m_audio_display;
    delete m_audio_widget;
    delete m_video_display;
    delete m_video_selector;
    delete m_controller;
}

SwitchSystemWidget::SwitchSystemWidget(
    QWidget& parent,
    SwitchSystemSession& session,
    uint64_t program_id
)
    : QWidget(&parent)
    , m_session(session)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);

    m_group_box = new CollapsibleGroupBox(*this, "Console " + QString::number(m_session.console_number()) + " Settings");
    layout->addWidget(m_group_box);

    QWidget* widget = new QWidget(m_group_box);
    m_group_box->set_widget(widget);
    {
        m_audio_display = new AudioDisplayWidget(*this, m_session.logger(), m_session.audio());
        layout->addWidget(m_audio_display);

        QVBoxLayout* video_holder = new QVBoxLayout();
        layout->addLayout(video_holder);
        video_holder->setContentsMargins(0, 0, 0, 0);

        m_video_display = new VideoDisplayWidget(
            *this, *video_holder,
            m_session.console_number(),
            m_session.video(),
            m_session.overlay()
        );
        video_holder->addWidget(m_video_display);
    }
    {
        QVBoxLayout* group_layout = new QVBoxLayout(widget);
        group_layout->setAlignment(Qt::AlignTop);
        group_layout->setContentsMargins(0, 0, 0, 0);

        m_controller = new ControllerSelectorWidget(*this, m_session.controller());
        group_layout->addWidget(m_controller);

        m_video_selector = new VideoSourceSelectorWidget(m_session.logger(), m_session.video());
        group_layout->addWidget(m_video_selector);

        m_audio_widget = new AudioSelectorWidget(*widget, m_session.audio());
        group_layout->addWidget(m_audio_widget);

#if 0
        //  Experiment with multiple controller layouts.
        m_controller = new ControllerSelectorWidget(*this, m_session.controller());
        group_layout->addWidget(m_controller);

        group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller()));
        group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller()));
        group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller()));
        group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller()));
        group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller()));
        group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller()));
        group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller()));
#endif

        m_command = new CommandRow(
            *widget,
            m_session,
            m_session.console_type(),
            m_session.allow_commands_while_running()
        );
        group_layout->addWidget(m_command);
    }

    setFocusPolicy(Qt::StrongFocus);


//    connect(
//        m_serial_widget, &SerialPortWidget::signal_on_ready,
//        m_command, [this](bool ready){
//            m_command->update_ui();
//        }
//    );
}


void SwitchSystemWidget::update_ui(ProgramState state){
    if (state != ProgramState::STOPPED){
        m_session.lock_controllers("Program is Running");
    }else{
        m_session.unlock_controllers();
    }
    m_command->on_state_changed(state);
}


void SwitchSystemWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent" << endl;
    QWidget::focusInEvent(event);
    m_session.overlay().report_focus_in();
}
void SwitchSystemWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent" << endl;
    QWidget::focusOutEvent(event);
    m_session.overlay().report_focus_out();
}
void SwitchSystemWidget::keyPressEvent(QKeyEvent* event){
//    cout << "SwitchSystemWidget::keyPressEvent()" << endl;
    m_session.overlay().report_key_press(event);
//    QWidget::keyPressEvent(event);
}
void SwitchSystemWidget::keyReleaseEvent(QKeyEvent* event){
//    cout << "SwitchSystemWidget::keyReleaseEvent()" << endl;
    m_session.overlay().report_key_release(event);
//    QWidget::keyReleaseEvent(event);
}



}
}
