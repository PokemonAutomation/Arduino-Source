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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


VideoDisplayWindow::VideoDisplayWindow(VideoDisplayWidget* display_widget)
    : QMainWindow(display_widget->parentWidget())
    , m_display_widget(display_widget)
{
    m_display_widget->setParent(this);
    setWindowTitle("Console: " + QString::number(m_display_widget->id()));

//    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->setCentralWidget(m_display_widget);
//    m_display_widget->setAlignment(Qt::AlignCenter);

//    cout << "Display Widget: " << m_display_widget->width() << " x " << m_display_widget->height() << endl;

    m_normal_size = m_display_widget->size();
    this->resize(m_normal_size);
//    cout << this->width() << " x " << this->height() << endl;

    this->show();
    this->raise(); // bring the window to front on macOS
    this->activateWindow(); // bring the window to front on Windows

    setFocusPolicy(Qt::StrongFocus);

//    this->showFullScreen();
}

void VideoDisplayWindow::changeEvent(QEvent* event){
    QMainWindow::changeEvent(event);
//    if (event->type() != QEvent::WindowStateChange){
//        QMainWindow::changeEvent(event);
//        return;
//    }
//    QWindowStateChangeEvent* change_event = static_cast<QWindowStateChangeEvent*>(event);
    if (this->windowState() == Qt::WindowMaximized){
        this->showFullScreen();
//        cout << "Display Widget: " << m_display_widget->width() << " x " << m_display_widget->height() << endl;
    }
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
    this->resize(m_display_widget->size());
}

void VideoDisplayWindow::exit_full_screen(){
    this->showNormal();
    QMetaObject::invokeMethod(this, [this]{
        this->resize(m_normal_size);
//        cout << "Display Widget: " << m_display_widget->width() << " x " << m_display_widget->height() << endl;
//        cout << "Window Widget: " << this->width() << " x " << this->height() << endl;
    }, Qt::QueuedConnection);
}

void VideoDisplayWindow::mouseDoubleClickEvent(QMouseEvent* event){
    if (this->isFullScreen()){
        exit_full_screen();
    }else{
        this->showFullScreen();
//        cout << "Display Widget: " << m_display_widget->width() << " x " << m_display_widget->height() << endl;
    }
//    QWidget::mouseDoubleClickEvent(event);
}

void VideoDisplayWindow::keyPressEvent(QKeyEvent* event){
    if ((Qt::Key)event->key() == Qt::Key::Key_Escape){
        exit_full_screen();
        return;
    }

    if (m_display_widget->command_receiver().key_press(event)){
        return;
    }
    QWidget::keyPressEvent(event);
}
void VideoDisplayWindow::keyReleaseEvent(QKeyEvent* event){
    if (m_display_widget->command_receiver().key_release(event)){
        return;
    }
    QWidget::keyReleaseEvent(event);
}
void VideoDisplayWindow::focusInEvent(QFocusEvent* event){
    m_display_widget->command_receiver().focus_in(event);
    QWidget::focusInEvent(event);
}
void VideoDisplayWindow::focusOutEvent(QFocusEvent* event){
    m_display_widget->command_receiver().focus_out(event);
    QWidget::focusOutEvent(event);
}

}
