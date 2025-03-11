/*  Collapsible Group Box
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QToolButton>
#include <QGroupBox>
#include "CollapsibleGroupBox.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


CollapsibleGroupBox::CollapsibleGroupBox(QWidget& parent, const QString& title, bool expanded)
    : QGroupBox(title, &parent)
    , m_widget(nullptr)
{
    this->setCheckable(true);
    this->setChecked(expanded);
    new QVBoxLayout(this);
    set_expanded(expanded);

    connect(
        this, &QGroupBox::toggled,
        this, [this](bool on){
            set_expanded(on);
        }
    );
}
void CollapsibleGroupBox::set_expanded(bool expanded){
    if (expanded){
        this->setFlat(false);
        this->layout()->setContentsMargins(6, 6, 6, 6);
    }else{
        this->setFlat(true);
        this->layout()->setContentsMargins(0, 0, 0, 0);
    }
    if (m_widget != nullptr){
        m_widget->setVisible(expanded);
    }
}

QWidget* CollapsibleGroupBox::widget(){
    return m_widget;
}
void CollapsibleGroupBox::set_widget(QWidget* widget){
    delete m_widget;
    if (widget != nullptr){
        widget->setParent(this);
        m_widget = widget;
        this->layout()->addWidget(widget);
    }
}



#if 0
CollapsibleGroupBox::CollapsibleGroupBox(QWidget& parent, const QString& title)
    : QWidget(&parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_arrow = new QToolButton(this);
    m_arrow->setStyleSheet("QToolButton {border: none;}");
    m_arrow->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_arrow->setArrowType(Qt::ArrowType::DownArrow);
    m_arrow->setText(title);
    m_arrow->setCheckable(true);
    m_arrow->setChecked(true);
    layout->addWidget(m_arrow);

    m_box = new QGroupBox(this);
    layout->addWidget(m_box);

    connect(
        m_arrow, &QToolButton::toggled,
        this, [m_arrow, m_box](bool on){
            m_arrow->setArrowType(on ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
            m_box->setVisible(on);
        }
    );
}
QWidget* CollapsibleGroupBox::box(){
    return m_box;
}
#endif





}
