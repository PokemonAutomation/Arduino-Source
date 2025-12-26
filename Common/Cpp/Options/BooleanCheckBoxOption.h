/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_BooleanCheckBoxOption_H
#define PokemonAutomation_Options_BooleanCheckBoxOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{



class BooleanCheckBoxCell : public ConfigOption{
public:
    ~BooleanCheckBoxCell();
    BooleanCheckBoxCell(const BooleanCheckBoxCell& x);
    BooleanCheckBoxCell(
        LockMode lock_while_running,
        bool default_value, bool current_value
    );


public:
    BooleanCheckBoxCell(
        LockMode lock_while_running,
        bool default_value
    );

    bool default_value() const;
    bool current_value() const;

    operator bool() const;
    void operator=(bool x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;


public:
    static ConfigUiFactory<BooleanCheckBoxCell> m_ui_factory;

    virtual UiWrapper make_UiComponent(void* params) override{
        return run_factory(m_ui_factory, *this, params);
    }


protected:
    struct Data;
    Pimpl<Data> m_data;
};




class BooleanCheckBoxOption : public BooleanCheckBoxCell{
public:
    BooleanCheckBoxOption(const BooleanCheckBoxOption& x) = delete;
    BooleanCheckBoxOption(
        std::string label,
        LockMode lock_while_running,
        bool default_value
    );
    BooleanCheckBoxOption(
        std::string label,
        LockMode lock_while_running,
        bool default_value, bool value
    );

    const std::string& label() const{ return m_label; }
    using BooleanCheckBoxCell::operator=;


public:
    static ConfigUiFactory<BooleanCheckBoxOption> m_ui_factory;

    virtual UiWrapper make_UiComponent(void* params) override{
        return run_factory(m_ui_factory, *this, params);
    }


private:
    const std::string m_label;
};





}
#endif

