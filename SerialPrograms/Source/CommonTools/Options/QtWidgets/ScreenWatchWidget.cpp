/*  Screen Watch Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QScreen>
#include <QMouseEvent>
#include <QApplication>
#include <QGuiApplication>
#include "ScreenWatchWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ConfigWidget* ScreenWatchDisplay::make_QtWidget(QWidget& parent){
    return new ScreenWatchWidget(*this, parent);
}
ConfigWidget* ScreenWatchButtons::make_QtWidget(QWidget& parent){
    return new ScreenWatchButtonWidget(m_option, parent);
}


ScreenWatchDisplayWidget::ScreenWatchDisplayWidget(ScreenWatchOption& option, ScreenWatchWidget& parent)
    : QWidget(&parent)
    , m_holder(parent)
    , m_option(option)
    , m_stop(false)
    , m_updater(&ScreenWatchDisplayWidget::thread_loop, this)
{}
ScreenWatchDisplayWidget::~ScreenWatchDisplayWidget(){
    {
        std::lock_guard lg(m_lock);
        m_stop = true;
        m_cv.notify_all();
    }
    m_updater.join();
}
void ScreenWatchDisplayWidget::paintEvent(QPaintEvent* event){
//    cout << "ScreenWatchDisplayWidget::paintEvent: " << m_holder.width() << " x " << m_holder.height() << endl;
    QWidget::paintEvent(event);

    VideoSnapshot snapshot = m_last_frame;
    if (!snapshot){
        return;
    }

    double aspect_ratio = (double)snapshot.frame->width() / snapshot.frame->height();
    if (aspect_ratio < 2.0){
//        cout << "ScreenWatchDisplayWidget::paintEvent: box " << m_holder.width() << " x " << m_holder.height() << endl;
        m_holder.set_all(WidgetStackFixedAspectRatio::EXPAND_TO_BOX, aspect_ratio);
        m_holder.setFixedHeight(m_holder.width() / 2);
    }else{
//        cout << "ScreenWatchDisplayWidget::paintEvent: adjust " << m_holder.width() << " x " << m_holder.height() << endl;
        m_holder.set_all(WidgetStackFixedAspectRatio::ADJUST_HEIGHT_TO_WIDTH, aspect_ratio);
    }

    QRect rect(0, 0, this->width(), this->height());
    QPainter painter(this);
    painter.drawImage(rect, snapshot.frame->to_QImage_ref());
}


void ScreenWatchDisplayWidget::thread_loop(){
    std::unique_lock lg(m_lock);
    while (!m_stop){
        m_last_frame = m_option.screenshot();
        QMetaObject::invokeMethod(this, [&]{
            this->update();
        }, Qt::QueuedConnection);
        m_cv.wait_for(lg, std::chrono::milliseconds(100));
    }
}

ScreenWatchWidget::ScreenWatchWidget(ScreenWatchDisplay& option, QWidget& parent)
    : WidgetStackFixedAspectRatio(parent, ADJUST_HEIGHT_TO_WIDTH)
    , ConfigWidget(option, *this)
{
    m_widget = new ScreenWatchDisplayWidget(option.m_option, *this);
    add_widget(*m_widget);
    m_overlay = new VideoOverlayWidget(*this, option.m_option.overlay());
    add_widget(*m_overlay);
}



class SelectorOverlay : public QDialog{
public:
    SelectorOverlay(ScreenWatchOption& option, QWidget& parent, QScreen& screen)
        : QDialog(&parent, Qt::FramelessWindowHint)
        , m_screen(screen.size())
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setGeometry(screen.geometry());
    }

    virtual void keyPressEvent(QKeyEvent* event) override{
//        cout << event->key() << endl;
        switch (event->key()){
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
            this->accept();
            break;
        default:
            QDialog::keyPressEvent(event);
        }
    }
    virtual void mousePressEvent(QMouseEvent* event) override{
//        cout << event->globalPos().x() << ", " << event->globalPos().y() << endl;
        m_mouse_down = true;
        m_selected_rect = QRect();
        m_selected_rect.setTopLeft(event->pos());
        m_selected_rect.setWidth(0);
        m_selected_rect.setHeight(0);
    }
    virtual void mouseReleaseEvent(QMouseEvent* event) override{
        m_mouse_down = false;
    }
    virtual void mouseMoveEvent(QMouseEvent* event) override{
//        cout << event->globalPos().x() << ", " << event->globalPos().y() << endl;
        if (m_mouse_down){
            m_selected_rect.setBottomRight(event->pos());
        }
        update();
    }
    virtual void paintEvent(QPaintEvent* event) override{
        QRect rect(0, 0, this->width(), this->height());
        QPainter painter(this);
        painter.fillRect(rect, QColor::fromRgb(255, 255, 255, 128));

        painter.setPen(Qt::red);
        painter.fillRect(m_selected_rect, QColor::fromRgb(255, 255, 0, 192));
//        cout << m_selected_rect.x() << ", " << m_selected_rect.y() << " - " << m_selected_rect.width() << ", " << m_selected_rect.height() << endl;
    }

    ImageFloatBox get_box() const{
        int x = m_selected_rect.x();
        int y = m_selected_rect.y();
        int w = m_selected_rect.width();
        int h = m_selected_rect.height();
        if (w < 0){
            x += w;
            w = -w;
        }
        if (h < 0){
            y += h;
            h = -h;
        }
        double inv_width = 1 / (double)m_screen.width();
        double inv_height = 1 / (double)m_screen.height();
        return {
            x * inv_width,
            y * inv_height,
            w * inv_width,
            h * inv_height,
        };
    }

private:
    QSize m_screen;
    bool m_mouse_down = false;
    QRect m_selected_rect;
};




ScreenWatchButtonWidget::ScreenWatchButtonWidget(ScreenWatchOption& option, QWidget& parent)
    : QWidget(&parent)
    , ConfigWidget(option, *this)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QPushButton* draw_box = new QPushButton("Draw Box", this);
    QPushButton* reset_button = new QPushButton("Reset to Full Screen", this);

    QHBoxLayout* buttons = new QHBoxLayout();
    layout->addLayout(buttons);
    buttons->addWidget(draw_box);
    buttons->addWidget(reset_button);

    layout->addWidget(new QLabel("Click on \"Draw Box\" and click+drag to select a region on the target monitor. Press ESC when done."));

    connect(
        draw_box, &QPushButton::clicked,
        this, [&](bool){
            qsizetype index = (qsizetype)option.MONITOR_INDEX;
            auto screens = QGuiApplication::screens();
            if (screens.size() <= index){
                return;
            }

            SelectorOverlay w(option, *this, *screens[index]);
            w.exec();

            ImageFloatBox box = w.get_box();
            if (box.width == 0 || box.height == 0){
                return;
            }

            option.X.set(box.x);
            option.Y.set(box.y);
            option.WIDTH.set(box.width);
            option.HEIGHT.set(box.height);
        }
    );

    connect(
        reset_button, &QPushButton::clicked,
        this, [&](bool){
            option.X.set(0.0);
            option.Y.set(0.0);
            option.WIDTH.set(1.0);
            option.HEIGHT.set(1.0);
        }
    );
}






}

