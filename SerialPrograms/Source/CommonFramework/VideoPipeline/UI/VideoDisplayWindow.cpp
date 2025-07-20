/*  Video Display Window
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QLayout>
#include <QMainWindow>
#include <QResizeEvent>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Windows/WindowTracker.h"
#include "VideoDisplayWidget.h"
#include "VideoDisplayWindow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


VideoDisplayWindow::VideoDisplayWindow(VideoDisplayWidget* display_widget)
    : m_display_widget(display_widget)
{
    setWindowTitle("Console: " + QString::number(m_display_widget->id()));
    this->setWindowIcon(QIcon(QString::fromStdString(RESOURCE_PATH() + "icon.png")));

    // Sets m_display_widget to be the VideoDisplayWindow's central widget.
    // VideoDisplayWindow now takes ownership of m_display_widget as a QWidget
    this->setCentralWidget(m_display_widget);
    
    // this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    // m_display_widget->setAlignment(Qt::AlignCenter);

    // cout << "Display Widget: " << m_display_widget->width() << " x " << m_display_widget->height() << endl;
    m_normal_size = m_display_widget->size();
    this->resize(m_normal_size);
    // cout << this->width() << " x " << this->height() << endl;

    this->show();
    this->raise(); // bring the window to front on macOS
    this->activateWindow(); // bring the window to front on Windows

    setFocusPolicy(Qt::StrongFocus);
    add_window(*this);
}
VideoDisplayWindow::~VideoDisplayWindow(){
    remove_window(*this);
    close();
}

void VideoDisplayWindow::changeEvent(QEvent* event){
    QMainWindow::changeEvent(event);
    if (this->windowState() == Qt::WindowMaximized){
        this->showFullScreen();
//        cout << "Display Widget: " << m_display_widget->width() << " x " << m_display_widget->height() << endl;
    }
}
void VideoDisplayWindow::closeEvent(QCloseEvent* event){
    close();
    QMainWindow::closeEvent(event);
}

void VideoDisplayWindow::resizeEvent(QResizeEvent* event){
    QMainWindow::resizeEvent(event);
}

void VideoDisplayWindow::close(){
    if (m_display_widget == nullptr){
        return;
    }
    m_display_widget->move_back_from_window();
    m_display_widget = nullptr;
}
void VideoDisplayWindow::exit_full_screen(){
    this->showNormal();
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
    if (m_display_widget){
        m_display_widget->command_receiver().key_press(event);
        return;
    }
    QWidget::keyPressEvent(event);
}
void VideoDisplayWindow::keyReleaseEvent(QKeyEvent* event){
    if (m_display_widget){
        m_display_widget->command_receiver().key_release(event);
        return;
    }
    QWidget::keyReleaseEvent(event);
}
void VideoDisplayWindow::focusInEvent(QFocusEvent* event){
    if (m_display_widget){
        m_display_widget->command_receiver().focus_in(event);
    }
    QWidget::focusInEvent(event);
}
void VideoDisplayWindow::focusOutEvent(QFocusEvent* event){
    if (m_display_widget){
        m_display_widget->command_receiver().focus_out(event);
    }
    QWidget::focusOutEvent(event);
}

}
