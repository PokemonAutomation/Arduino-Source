/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MultiHostTable_H
#define PokemonAutomation_PokemonSwSh_MultiHostTable_H

#include "Common/Qt/Options/MultiHostTableOptionBase.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiHostTableOption : public ConfigOption{
public:
    MultiHostTableOption()
        : m_factory(true)
        , m_table("<b>Game List:</b>", m_factory, true)
    {}
    virtual void load_json(const QJsonValue& json) override{
        m_table.load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return m_table.write_current();
    }

    size_t size() const{
        return m_table.size();
    }
    const MultiHostSlot& operator[](size_t index) const{
        return static_cast<const MultiHostSlot&>(m_table[index]);
    }

    virtual QString check_validity() const override{
        return m_table.check_validity();
    }
    virtual void restore_defaults() override{
        m_table.restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class MultiHostTableOptionUI;
    MultiHostSlotOptionFactory m_factory;
    EditableTableBase m_table;
};
class MultiHostTableOptionUI : public EditableTableBaseUI, public ConfigOptionUI{
public:
    MultiHostTableOptionUI(QWidget& parent, MultiHostTableOption& value)
        : EditableTableBaseUI(parent, value.m_table)
        , ConfigOptionUI(value, *this)
    {}
    virtual void restore_defaults() override{
        EditableTableBaseUI::restore_defaults();
    }

};
inline ConfigOptionUI* MultiHostTableOption::make_ui(QWidget& parent){
    return new MultiHostTableOptionUI(parent, *this);
}




}
}
}
#endif
