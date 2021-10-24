/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EditableTableOption_H
#define PokemonAutomation_EditableTableOption_H

#include "Common/Qt/Options/EditableTableOptionBase.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class EditableTableOption : public ConfigOption, public EditableTableBase{
public:
    EditableTableOption(QString label, const EditableTableFactory& factory, bool margin)
        : EditableTableBase(std::move(label), factory, margin)
    {}

    virtual void load_json(const QJsonValue& json) override{
        return EditableTableBase::load_current(json);
    }
    virtual QJsonValue to_json() const override{
        return EditableTableBase::write_current();
    }

    virtual QString check_validity() const override{
        return EditableTableBase::check_validity();
    };
    virtual void restore_defaults() override{
        EditableTableBase::restore_defaults();
    };

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;
};


class EditableTableOptionUI : public ConfigOptionUI, public EditableTableBaseUI{
public:
    EditableTableOptionUI(QWidget& parent, EditableTableOption& value)
        : EditableTableBaseUI(parent, value)
    {}
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{
        EditableTableBaseUI::restore_defaults();
    }
};


inline ConfigOptionUI* EditableTableOption::make_ui(QWidget& parent){
    return new EditableTableOptionUI(parent, *this);
}




}
#endif
