/*  Group Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include "Common/Compiler.h"
#include "GroupWidget.h"

namespace PokemonAutomation{



ConfigWidget* GroupOption::make_QtWidget(QWidget& parent){
    return new GroupWidget(parent, *this);
}


GroupWidget::~GroupWidget(){
    m_value.remove_listener(*this);
}
GroupWidget::GroupWidget(QWidget& parent, GroupOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    m_group_box = new QGroupBox(QString::fromStdString(value.m_label), this);
    m_group_box->setCheckable(value.m_toggleable);
    m_group_box->setChecked(value.enabled());
    layout->addWidget(m_group_box);

#if 0
    QVBoxLayout* mid_layout = new QVBoxLayout(group_box);
    QWidget* mid_widget = new QWidget(group_box);
    mid_layout->addWidget(mid_widget);
    mid_layout-setContentsMargins(0, 0, 0, 0);

    QFont font = group_box->font();
    mid_widget->setFont(font);
    font.setBold(true);
    group_box->setFont(font);
#endif

    QVBoxLayout* group_layout = new QVBoxLayout(m_group_box);
    group_layout->setAlignment(Qt::AlignTop);
    group_layout->setContentsMargins(0, 10, 0, 0);

    m_expand_text = new QWidget(m_group_box);
    m_expand_text->setLayout(new QVBoxLayout());
    m_expand_text->layout()->addWidget(new QLabel("(double click to expand)", this));
    m_expand_text->setVisible(false);
    group_layout->addWidget(m_expand_text);


    m_options_holder = new QWidget(m_group_box);
    group_layout->addWidget(m_options_holder);
    m_options_layout = new QVBoxLayout(m_options_holder);
    m_options_layout->setContentsMargins(0, 0, 0, 0);

    for (auto& item : value.options()){
        m_options.emplace_back(item->make_QtWidget(parent));
        m_options.back()->widget().setContentsMargins(5, 5, 5, 5);
        m_options_layout->addWidget(&m_options.back()->widget());
    }

    connect(
        m_group_box, &QGroupBox::toggled,
        this, [this](bool on){
            m_value.m_enabled.store(on, std::memory_order_relaxed);
            m_value.on_set_enabled(on);
        }
    );

    value.add_listener(*this);
}
void GroupWidget::set_options_enabled(bool enabled){
    for (ConfigWidget* item : m_options){
        item->widget().setEnabled(enabled);
    }
}
void GroupWidget::update(){
    ConfigWidget::update();
    bool on = m_value.m_enabled.load(std::memory_order_acquire);
    m_group_box->setChecked(on);
    for (ConfigWidget* item : m_options){
        item->update();
    }
}
void GroupWidget::value_changed(){
    QMetaObject::invokeMethod(this, [this]{
        update();
    }, Qt::QueuedConnection);
}
void GroupWidget::mouseDoubleClickEvent(QMouseEvent* event){
    m_expand_text->setVisible(m_expanded);
    m_expanded = !m_expanded;
    m_options_holder->setVisible(m_expanded);
}




}
