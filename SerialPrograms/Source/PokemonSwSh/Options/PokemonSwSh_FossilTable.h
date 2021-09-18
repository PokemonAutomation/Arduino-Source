/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FossilTable_H
#define PokemonAutomation_PokemonSwSh_FossilTable_H

#include "Common/Qt/Options/FossilTableOptionBase.h"
#include "CommonFramework/Options/ConfigOption.h"

namespace PokemonAutomation{



class FossilTableOption : public ConfigOption, public FossilTableOptionBase{
public:
    FossilTableOption()
        : FossilTableOptionBase("<b>Game List:</b>")
    {}
    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual bool is_valid() const override{
        return FossilTableOptionBase::is_valid();
    }
    virtual void restore_defaults() override{
        FossilTableOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class FossilTableOptionUI : public ConfigOptionUI, public FossilTableOptionBaseUI{
public:
    FossilTableOptionUI(QWidget& parent, FossilTableOption& value)
        : FossilTableOptionBaseUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        FossilTableOptionBaseUI::restore_defaults();
    }
};

inline ConfigOptionUI* FossilTableOption::make_ui(QWidget& parent){
    return new FossilTableOptionUI(parent, *this);
}


}
#endif
