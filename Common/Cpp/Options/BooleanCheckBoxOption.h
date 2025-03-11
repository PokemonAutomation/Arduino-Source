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

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

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

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};





}
#endif

