/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "EnumDropdownWidget.h"

namespace PokemonAutomation{


ConfigWidget* IntegerEnumDropdownCell::make_ui(QWidget& parent){
    return new EnumDropdownCellWidget(parent, *this);
}
ConfigWidget* IntegerEnumDropdownOption::make_ui(QWidget& parent){
    return new EnumDropdownOptionWidget(parent, *this);
}



EnumDropdownCellWidget::~EnumDropdownCellWidget(){
    m_value.remove_listener(*this);
}
EnumDropdownCellWidget::EnumDropdownCellWidget(QWidget& parent, IntegerEnumDropdownCell& value)
    : NoWheelComboBox(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    for (const auto& item : value.database().all_values()){
        int index = this->count();
        m_value_to_index[item] = index;
        m_index_to_value.emplace_back(item);

        const EnumEntry& entry = *value.database().find(item);

        this->addItem(QString::fromStdString(entry.display));
        if (entry.enabled){
            continue;
        }
        auto* model = qobject_cast<QStandardItemModel*>(this->model());
        if (model == nullptr){
            continue;
        }
        QStandardItem* line_handle = model->item(this->count() - 1);
        if (line_handle != nullptr){
            line_handle->setEnabled(false);
        }
    }
    this->setCurrentIndex(m_value_to_index[m_value.current_value()]);

    connect(
        this, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index){
            if (index < 0 || (size_t)index >= m_index_to_value.size()){
                m_value.restore_defaults();
                return;
            }
            m_value.set_value(m_index_to_value[index]);
        }
    );

    m_value.add_listener(*this);
}


void EnumDropdownCellWidget::update(){
    ConfigWidget::update();
    this->setCurrentIndex(m_value_to_index[m_value.current_value()]);
}
void EnumDropdownCellWidget::value_changed(){
    QMetaObject::invokeMethod(this, [=]{
        update();
    }, Qt::QueuedConnection);
}





EnumDropdownOptionWidget::EnumDropdownOptionWidget(QWidget& parent, IntegerEnumDropdownOption& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_cell(new EnumDropdownCellWidget(*this, value))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    QLabel* text = new QLabel(QString::fromStdString(value.label()), this);
    text->setWordWrap(true);
    layout->addWidget(text, 1);
    layout->addWidget(m_cell, 1);
}


void EnumDropdownOptionWidget::update(){
    ConfigWidget::update();
    m_cell->update();
}







}
