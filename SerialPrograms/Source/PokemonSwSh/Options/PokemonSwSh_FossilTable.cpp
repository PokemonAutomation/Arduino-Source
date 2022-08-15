/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
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
    virtual void update() override{
        ConfigWidget::update();
        EditableTableBaseWidget::update_ui();
    }

};



FossilTableOption::FossilTableOption()
    : m_table("<b>Game List:</b>", m_factory)
{}
void FossilTableOption::load_json(const JsonValue& json){
    m_table.load_current(json);
}
JsonValue FossilTableOption::to_json() const{
    return m_table.write_current();
}

size_t FossilTableOption::size() const{
    return m_table.size();
}
const FossilGame& FossilTableOption::operator[](size_t index) const{
    return static_cast<const FossilGame&>(m_table[index]);
}

std::string FossilTableOption::check_validity() const{
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
