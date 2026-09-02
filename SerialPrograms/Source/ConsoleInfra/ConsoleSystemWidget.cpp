/*  Console System (Qt Widget)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QGroupBox>
#include "Controllers/ControllerSelectorWidget.h"
#include "CommandRowWidget.h"
#include "ConsoleSystemWidget.h"

namespace PokemonAutomation{
namespace ConsoleInfra{



ConsoleSystemWidget::~ConsoleSystemWidget(){
    //  Delete all the UI elements first since they reference the states.
    delete m_audio_display;
    delete m_audio_widget;
    delete m_video_display;
    delete m_video_selector;
}

ConsoleSystemWidget::ConsoleSystemWidget(
    QWidget& parent,
    ConsoleSystemSession& session,
    bool include_command_row
)
    : QWidget(&parent)
    , m_session(session)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);

    m_group_box = new CollapsibleGroupBox(*this, "Console " + QString::number(m_session.console_number()) + " Settings");
    layout->addWidget(m_group_box);

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

    QWidget* widget = new QWidget(m_group_box);
    m_group_box->set_widget(widget);
    m_group_layout = new QVBoxLayout(widget);
    m_group_layout->setAlignment(Qt::AlignTop);
    m_group_layout->setContentsMargins(0, 0, 0, 0);

    if (session.controllers() == 1){
        m_group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller(0), std::nullopt));
    }

    m_video_selector = new VideoSourceSelectorWidget(m_session.logger(), m_session.video());
    m_group_layout->addWidget(m_video_selector);

    m_audio_widget = new AudioSelectorWidget(*m_group_box->widget(), m_session.audio());
    m_group_layout->addWidget(m_audio_widget);

    if (session.controllers() != 1){
        for (size_t c = 0; c < m_session.controllers(); c++){
            m_group_layout->addWidget(new ControllerSelectorWidget(*this, m_session.controller(c), c));
        }
    }

    if (include_command_row){
        m_group_layout->addWidget(new CommandRowWidget(*this, m_session));
    }
}


void ConsoleSystemWidget::focusInEvent(QFocusEvent* event){
//    cout << "focusInEvent" << endl;
    QWidget::focusInEvent(event);
    m_session.overlay().report_focus_in();
}
void ConsoleSystemWidget::focusOutEvent(QFocusEvent* event){
//    cout << "focusOutEvent" << endl;
    QWidget::focusOutEvent(event);
    m_session.overlay().report_focus_out();
}
void ConsoleSystemWidget::keyPressEvent(QKeyEvent* event){
//    cout << "SwitchSystemWidget::keyPressEvent()" << endl;
    m_session.overlay().report_key_press(event);
//    QWidget::keyPressEvent(event);
}
void ConsoleSystemWidget::keyReleaseEvent(QKeyEvent* event){
//    cout << "SwitchSystemWidget::keyReleaseEvent()" << endl;
    m_session.overlay().report_key_release(event);
//    QWidget::keyReleaseEvent(event);
}







}
}
