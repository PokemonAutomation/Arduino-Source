/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "Common/Qt/Options/EditableTable/EditableTableBaseWidget.h"
#include "PokemonSwSh_FossilTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




class FossilTableWidget : public EditableTableBaseWidget, public ConfigWidget{
public:
    FossilTableWidget(QWidget& parent, FossilTableOption& value)
        : EditableTableBaseWidget(parent, value.m_table)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        EditableTableBaseWidget::restore_defaults();
    }

};



FossilTableOption::FossilTableOption()
    : m_table("<b>Game List:</b>", m_factory, true)
{}
void FossilTableOption::load_json(const QJsonValue& json){
    m_table.load_current(json);
}
QJsonValue FossilTableOption::to_json() const{
    return m_table.write_current();
}

size_t FossilTableOption::size() const{
    return m_table.size();
}
const FossilGame& FossilTableOption::operator[](size_t index) const{
    return static_cast<const FossilGame&>(m_table[index]);
}

QString FossilTableOption::check_validity() const{
    return m_table.check_validity();
}
void FossilTableOption::restore_defaults(){
    m_table.restore_defaults();
}


ConfigWidget* FossilTableOption::make_ui(QWidget& parent){
    return new FossilTableWidget(parent, *this);
}



}
}
}
