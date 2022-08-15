/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "Common/Qt/NoWheelComboBox.h"
#include "EnumDropdownWidget.h"

namespace PokemonAutomation{



ConfigWidget* EnumDropdownOption::make_ui(QWidget& parent){
    return new EnumDropdownWidget(parent, *this);
}


EnumDropdownWidget::~EnumDropdownWidget(){
    m_value.remove_listener(*this);
}
EnumDropdownWidget::EnumDropdownWidget(QWidget& parent, EnumDropdownOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(m_value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    m_box = new NoWheelComboBox(&parent);
    layout->addWidget(m_box);

    for (const auto& item : m_value.case_list()){
        m_box->addItem(QString::fromStdString(item.name));
        if (item.enabled){
            continue;
        }
        auto* model = qobject_cast<QStandardItemModel*>(m_box->model());
        if (model == nullptr){
            continue;
        }
        QStandardItem* line_handle = model->item(m_box->count() - 1);
        if (line_handle != nullptr){
            line_handle->setEnabled(false);
        }
    }
    m_box->setCurrentIndex((int)m_value);
    layout->addWidget(m_box, 1);

    connect(
        m_box, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0){
                m_value.restore_defaults();
                return;
            }
            m_value.set(index);
        }
    );

    m_value.add_listener(*this);
}


void EnumDropdownWidget::update(){
    ConfigWidget::update();
    m_box->setCurrentIndex((int)m_value);
}
void EnumDropdownWidget::value_changed(){
    QMetaObject::invokeMethod(m_box, [=]{
        update();
    }, Qt::QueuedConnection);
}





}
