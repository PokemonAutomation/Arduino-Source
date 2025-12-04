/*  Checkbox Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CheckboxDropdownWidget.h"

namespace PokemonAutomation{


ConfigWidget* CheckboxDropdownBase::make_QtWidget(QWidget& parent){
    return new CheckboxDropdownCellWidget(parent, *this);
}




CheckboxDropdownCellWidget::~CheckboxDropdownCellWidget(){
    m_value.remove_listener(*this);
}
CheckboxDropdownCellWidget::CheckboxDropdownCellWidget(QWidget& parent, CheckboxDropdownBase& value)
    : CheckboxDropdown(&parent, QString::fromStdString(value.current_label()))
    , ConfigWidget(value, *this)
    , m_value(value)
{
    size_t total_items = value.size();
    for (size_t index = 0; index < total_items; index++){
        CheckboxDropdownItem* item = addItem(QString::fromStdString(value.name_at_index(index)));
        connect(
            item, &CheckboxDropdownItem::checkStateChanged,
            this, [&value, index](Qt::CheckState state){
                if (state == Qt::Checked){
                    value.set_index(index);
                }else{
                    value.clear_index(index);
                }
            }
        );
    }

    CheckboxDropdownCellWidget::update_value();

    m_value.add_listener(*this);
}

void CheckboxDropdownCellWidget::update_value(){
    setLabel(QString::fromStdString(m_value.current_label()));
    size_t total_items = CheckboxDropdown::size();
    for (size_t index = 0; index < total_items; index++){
        (*this)[index]->setChecked(m_value[index]);
    }
}
void CheckboxDropdownCellWidget::on_config_value_changed(void* object){
    QMetaObject::invokeMethod(this, [this]{
        update_value();
    }, Qt::QueuedConnection);
}



}
