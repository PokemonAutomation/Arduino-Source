/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include "BatchWidget.h"

namespace PokemonAutomation{



ConfigWidget* BatchOption::make_QtWidget(QWidget& parent){
    return new BatchWidget(parent, *this);
}



BatchWidget::~BatchWidget(){
    m_value.remove_listener(*this);
}
BatchWidget::BatchWidget(QWidget& parent, BatchOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QBoxLayout* options_layout;
    if (value.horizontal()){
        options_layout = new QHBoxLayout(this);
        options_layout->setAlignment(Qt::AlignLeft);
        options_layout->setContentsMargins(0, 0, 0, 0);
    }else{
        options_layout = new QVBoxLayout(this);
        options_layout->setAlignment(Qt::AlignTop);
        options_layout->setContentsMargins(0, 0, 0, 0);
    }

    for (auto& item : value.options()){
        m_options.emplace_back(item->make_QtWidget(parent));
        if (value.horizontal()){
            m_options.back()->widget().setContentsMargins(3, 0, 3, 0);
        }else{
            m_options.back()->widget().setContentsMargins(0, 3, 0, 3);
        }
        options_layout->addWidget(&m_options.back()->widget(), 0);
    }

    value.add_listener(*this);
}
void BatchWidget::update_value(){
    for (ConfigWidget* item : m_options){
        item->update_value();
    }
}
void BatchWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}



}
