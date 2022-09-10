/*  Video Display Window
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QLayout>
#include <QMainWindow>
#include <QResizeEvent>

#include "VideoDisplayWidget.h"
#include "VideoDisplayWindow.h"

#include "NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


VideoDisplayWindow::VideoDisplayWindow(VideoDisplayWidget* display_widget)
: QMainWindow(display_widget->parentWidget())
, m_display_widget(display_widget)
, m_parent_switch_system_widget(dynamic_cast<NintendoSwitch::SwitchSystemWidget*>(display_widget->parentWidget())){
    m_display_widget->setParent(this);
    this->setCentralWidget(m_display_widget);
    this->resize(m_display_widget->width(), m_display_widget->height());

    this->show();
    this->raise(); // bring the window to front on macOS
    this->activateWindow(); // bring the window to front on Windows

    setFocusPolicy(Qt::StrongFocus);
    
    // this->showFullScreen();
}

void VideoDisplayWindow::closeEvent(QCloseEvent* event){
    QWidget* parent = this->parentWidget();
    m_display_widget->setParent(parent);
    if (parent->layout()){
        parent->layout()->addWidget(m_display_widget);
    }
    QMainWindow::closeEvent(event);
}

void VideoDisplayWindow::resizeEvent(QResizeEvent* event){
    QMainWindow::resizeEvent(event);
}

void VideoDisplayWindow::keyPressEvent(QKeyEvent* event){
    if (m_parent_switch_system_widget){
        m_parent_switch_system_widget->key_press(event);
    }
    QWidget::keyPressEvent(event);
}
void VideoDisplayWindow::keyReleaseEvent(QKeyEvent* event){
    if (m_parent_switch_system_widget){
        m_parent_switch_system_widget->key_release(event);
    }
    QWidget::keyReleaseEvent(event);
}
void VideoDisplayWindow::focusInEvent(QFocusEvent* event){
    if (m_parent_switch_system_widget){
        m_parent_switch_system_widget->focus_in(event);
    }
    QWidget::focusInEvent(event);
}
void VideoDisplayWindow::focusOutEvent(QFocusEvent* event){
    if (m_parent_switch_system_widget){
        m_parent_switch_system_widget->focus_out(event);
    }
    QWidget::focusOutEvent(event);
}

}
