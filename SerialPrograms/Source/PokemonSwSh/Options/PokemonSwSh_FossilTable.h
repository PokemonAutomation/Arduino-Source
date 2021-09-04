/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FossilTable_H
#define PokemonAutomation_PokemonSwSh_FossilTable_H

#include "Common/Qt/Options/FossilTableOption.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{



class FossilTable : public ConfigOption, public FossilTableOption{
public:
    FossilTable()
        : FossilTableOption("<b>Game List:</b>")
    {}
    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual bool is_valid() const override{
        return FossilTableOption::is_valid();
    }
    virtual void restore_defaults() override{
        FossilTableOption::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class FossilTableUI : public ConfigOptionUI, public FossilTableOptionUI{
public:
    FossilTableUI(QWidget& parent, FossilTable& value)
        : FossilTableOptionUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        FossilTableOptionUI::restore_defaults();
    }
};

inline ConfigOptionUI* FossilTable::make_ui(QWidget& parent){
    return new FossilTableUI(parent, *this);
}


}
#endif
