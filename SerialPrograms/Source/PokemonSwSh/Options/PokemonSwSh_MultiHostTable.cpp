/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/EditableTable/EditableTableBaseWidget.h"
#include "PokemonSwSh_MultiHostTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class MultiHostTableWidget : public EditableTableBaseWidget, public ConfigWidget{
public:
    MultiHostTableWidget(QWidget& parent, MultiHostTableOption& value)
        : EditableTableBaseWidget(parent, value.m_table)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        EditableTableBaseWidget::restore_defaults();
    }
    virtual void update_ui() override{
        EditableTableBaseWidget::update_ui();
    }

};


MultiHostTableOption::MultiHostTableOption()
    : m_factory(true)
    , m_table("<b>Game List:</b>", m_factory)
{}
void MultiHostTableOption::load_json(const JsonValue& json){
    m_table.load_current(json);
}
JsonValue MultiHostTableOption::to_json() const{
    return m_table.write_current();
}

size_t MultiHostTableOption::size() const{
    return m_table.size();
}
const MultiHostSlot& MultiHostTableOption::operator[](size_t index) const{
    return static_cast<const MultiHostSlot&>(m_table[index]);
}

std::string MultiHostTableOption::check_validity() const{
    return m_table.check_validity();
}
void MultiHostTableOption::restore_defaults(){
    m_table.restore_defaults();
}


ConfigWidget* MultiHostTableOption::make_ui(QWidget& parent){
    return new MultiHostTableWidget(parent, *this);
}



}
}
}
