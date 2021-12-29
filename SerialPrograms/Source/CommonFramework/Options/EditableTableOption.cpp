/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "Common/Qt/Options/EditableTable/EditableTableBaseWidget.h"
#include "EditableTableOption.h"

namespace PokemonAutomation{



class EditableTableWidget : public EditableTableBaseWidget, public ConfigWidget{
public:
    EditableTableWidget(QWidget& parent, EditableTableOption& value)
        : EditableTableBaseWidget(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        EditableTableBaseWidget::restore_defaults();
    }
};



EditableTableOption::EditableTableOption(
    QString label, const EditableTableFactory& factory, bool margin,
    std::vector<std::unique_ptr<EditableTableRow>> default_value
)
    : EditableTableBaseOption(std::move(label), factory, margin, std::move(default_value))
{}

void EditableTableOption::load_json(const QJsonValue& json){
    return EditableTableBaseOption::load_current(json);
}
QJsonValue EditableTableOption::to_json() const{
    return EditableTableBaseOption::write_current();
}

QString EditableTableOption::check_validity() const{
    return EditableTableBaseOption::check_validity();
};
void EditableTableOption::restore_defaults(){
    EditableTableBaseOption::restore_defaults();
};





ConfigWidget* EditableTableOption::make_ui(QWidget& parent){
    return new EditableTableWidget(parent, *this);
}




}
