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



class MultiHostTableOption : public ConfigOption, public MultiHostTableOptionBase{
public:
    MultiHostTableOption()
        : MultiHostTableOptionBase("<b>Game List:</b>")
    {}
    virtual void load_json(const QJsonValue& json) override{
        load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return write_current();
    }

    virtual bool is_valid() const override{
        return MultiHostTableOptionBase::is_valid();
    }
    virtual void restore_defaults() override{
        MultiHostTableOptionBase::restore_defaults();
    }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class MultiHostTableOptionUI : public ConfigOptionUI, public MultiHostTableOptionBaseUI{
public:
    MultiHostTableOptionUI(QWidget& parent, MultiHostTableOption& value)
        : MultiHostTableOptionBaseUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        MultiHostTableOptionBaseUI::restore_defaults();
    }
};

inline ConfigOptionUI* MultiHostTableOption::make_ui(QWidget& parent){
    return new MultiHostTableOptionUI(parent, *this);
}


}
#endif
