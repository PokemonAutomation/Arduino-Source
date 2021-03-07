/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MultiHostTable_H
#define PokemonAutomation_MultiHostTable_H

#include "Common/Qt/Options/MultiHostTableOption.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{



class MultiHostTable : public ConfigOption, public MultiHostTableOption{
public:
    MultiHostTable()
        : ConfigOption("<b>Game List:</b>")
        , MultiHostTableOption(ConfigOption::m_label)
    {}
    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual bool is_valid() const override{
        return MultiHostTableOption::is_valid();
    }
    virtual void restore_defaults() override{
        MultiHostTableOption::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class MultiHostTableUI : public ConfigOptionUI, public MultiHostTableOptionUI{
public:
    MultiHostTableUI(QWidget& parent, MultiHostTable& value)
        : MultiHostTableOptionUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        MultiHostTableOptionUI::restore_defaults();
    }
};

inline ConfigOptionUI* MultiHostTable::make_ui(QWidget& parent){
    return new MultiHostTableUI(parent, *this);
}


}
#endif
