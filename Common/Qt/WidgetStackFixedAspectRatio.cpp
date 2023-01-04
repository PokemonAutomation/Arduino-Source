/*  Widget Stack Fixed Aspect Ratio
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QResizeEvent>
#include <QVBoxLayout>
#include "WidgetStackFixedAspectRatio.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



WidgetStackFixedAspectRatio::WidgetStackFixedAspectRatio(QWidget& parent, SizePolicy size_policy, double aspect_ratio)
    : QWidget(&parent)
    , m_size_policy(size_policy)
    , m_aspect_ratio(sanitize_aspect_ratio(aspect_ratio))
{
    m_detached_internal = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_detached_internal);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    m_stack_holder = new QWidget(m_detached_internal);
    layout->addWidget(m_stack_holder);
//    this->setFixedHeight(495);
}

void WidgetStackFixedAspectRatio::add_widget(QWidget& widget){
    widget.setParent(m_stack_holder);
    m_widgets.insert(&widget);
}
double WidgetStackFixedAspectRatio::sanitize_aspect_ratio(double aspect_ratio){
    if (aspect_ratio == 0 || std::isnan(aspect_ratio)){
        aspect_ratio = 16/9.;
    }
    aspect_ratio = std::min(aspect_ratio, 1000.);
    aspect_ratio = std::max(aspect_ratio, 0.001);
    return aspect_ratio;
}
void WidgetStackFixedAspectRatio::set_size_policy(SizePolicy size_policy){
    m_size_policy = size_policy;
    clear_fixed_dimensions();
    m_debouncer.clear();
    update_size(this->size());
}
void WidgetStackFixedAspectRatio::set_aspect_ratio(double aspect_ratio){
//    cout << "WidgetStackFixedAspectRatio::set_aspect_ratio(): " << aspect_ratio << endl;
    m_aspect_ratio = sanitize_aspect_ratio(aspect_ratio);
    m_debouncer.clear();
    update_size(this->size());
}

void WidgetStackFixedAspectRatio::clear_fixed_dimensions(){
    this->setMinimumSize(QSize(0, 0));
    this->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
}
void WidgetStackFixedAspectRatio::resize_to_box(QSize enclosing_box){
    int width, height;

    double box_ratio = (double)enclosing_box.width() / enclosing_box.height();
    if (m_aspect_ratio < box_ratio){
        height = enclosing_box.height();
        width = (int)(height * m_aspect_ratio);
    }else{
        width = enclosing_box.width();
        height = (int)(width / m_aspect_ratio);
    }

    QSize size(width, height);

    m_detached_internal->setFixedSize(enclosing_box);
    m_stack_holder->setFixedSize(size);
    for (QWidget* widget : m_widgets){
        widget->setFixedSize(size);
    }
}
void WidgetStackFixedAspectRatio::resize_to_width(int width){
    int previous_width = m_stack_holder->width();
//    cout << "WidgetStackFixedAspectRatio::resize_to_width(): " << width << " <- " << previous_width << endl;

    if (width > previous_width && width < previous_width + 50 && !m_debouncer.check(width)){
//        cout << "Supressing potential infinite resizing loop." << endl;
        return;
    }


    int height = (int)(width / m_aspect_ratio);
//    cout << "Resizing: " << width << " x " << height << " from: " << previous_width << " x " << m_stack_holder->height() << endl;

    if (width == previous_width && height == m_stack_holder->height()){
        return;
    }

    QSize size(width, height);
    this->setFixedHeight(height);

    m_detached_internal->setFixedSize(size);
    m_stack_holder->setFixedSize(size);
    for (QWidget* widget : m_widgets){
        widget->setFixedSize(size);
    }
}
void WidgetStackFixedAspectRatio::update_size(QSize size){
    switch (m_size_policy){
    case EXPAND_TO_BOX:
        resize_to_box(size);
        return;
    case ADJUST_HEIGHT_TO_WIDTH:
        resize_to_width(size.width());
        return;
    }
}
void WidgetStackFixedAspectRatio::resizeEvent(QResizeEvent* event){
//    cout << "WidgetStackFixedAspectRatio::resizeEvent(): " << event->size().width() << " x " << event->size().height() << endl;
    update_size(event->size());
}



}
