/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_Options_SimpleIntegerOption_H
#define PokemonAutomation_Options_SimpleIntegerOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{



template <typename Type>
class SimpleIntegerCell : public ConfigOption{
public:
    ~SimpleIntegerCell();
    SimpleIntegerCell(const SimpleIntegerCell& x);
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type min_value, Type max_value,
        Type default_value, Type current_value
    );

public:
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type default_value
    );
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type default_value, Type min_value
    );
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type default_value, Type min_value, Type max_value
    );

    Type min_value() const;
    Type max_value() const;
    Type default_value() const;
    Type current_value() const;

    operator Type() const;
    std::string set(Type x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity(Type x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

public:
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

protected:
    struct Data;
    Pimpl<Data> m_data;
};





template <typename Type>
class SimpleIntegerOption : public SimpleIntegerCell<Type>{
public:
    SimpleIntegerOption(const SimpleIntegerOption& x) = delete;
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type min_value, Type max_value,
        Type default_value, Type current_value
    );

public:
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type default_value
    );
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type default_value, Type min_value
    );
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type default_value, Type min_value, Type max_value
    );

    const std::string& label() const{ return m_label; }

public:
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};




}
#endif

