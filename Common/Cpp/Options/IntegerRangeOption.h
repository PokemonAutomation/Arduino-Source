/*  Integer Range Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_Options_IntegerRangeOption_H
#define PokemonAutomation_Options_IntegerRangeOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


template <typename Type>
class IntegerRangeCell : public ConfigOption{
public:
    ~IntegerRangeCell();
    IntegerRangeCell(const IntegerRangeCell& x);
    IntegerRangeCell(
        LockMode lock_while_running,
        Type lo_min_value, Type lo_max_value, Type lo_default_value, Type lo_current_value,
        Type hi_min_value, Type hi_max_value, Type hi_default_value, Type hi_current_value
    );

    Type lo_min_value() const;
    Type lo_max_value() const;
    Type lo_default_value() const;

    Type hi_min_value() const;
    Type hi_max_value() const;
    Type hi_default_value() const;

    Type lo_current_value() const;
    Type hi_current_value() const;
    void current_values(Type& lo, Type& hi) const;

    virtual void set_lo(Type x);
    virtual void set_hi(Type x);
    virtual void set(Type lo, Type hi);
    virtual void set(const IntegerRangeCell& option);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

//    std::string check_validity(Type x) const;
//    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

public:
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

protected:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif

