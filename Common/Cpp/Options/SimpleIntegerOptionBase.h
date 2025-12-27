/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_Options_SimpleIntegerOptionBase_H
#define PokemonAutomation_Options_SimpleIntegerOptionBase_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


class SimpleIntegerCellBase : public ConfigOptionImpl<SimpleIntegerCellBase>{
public:
    using NativeType = int64_t;

public:
    ~SimpleIntegerCellBase();
    SimpleIntegerCellBase(const SimpleIntegerCellBase& x);
    SimpleIntegerCellBase(
        LockMode lock_while_running,
        NativeType min_value, NativeType max_value,
        NativeType default_value, NativeType current_value
    );


public:
    SimpleIntegerCellBase(
        LockMode lock_while_running,
        NativeType default_value
    );
    SimpleIntegerCellBase(
        LockMode lock_while_running,
        NativeType default_value, NativeType min_value
    );
    SimpleIntegerCellBase(
        LockMode lock_while_running,
        NativeType default_value, NativeType min_value, NativeType max_value
    );

    NativeType min_value() const;
    NativeType max_value() const;
    NativeType default_value() const;
    NativeType current_value() const;

//    operator NativeType() const;
    std::string set(NativeType x);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity(NativeType x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;


protected:
    template <typename Type>
    static Type sanitize(Type x){
        return x;
    }
    static uint64_t sanitize(uint64_t x);


protected:
    struct Data;
    Pimpl<Data> m_data;
};





class SimpleIntegerOptionBase : public ConfigOptionImpl<SimpleIntegerOptionBase, SimpleIntegerCellBase>{
public:
    SimpleIntegerOptionBase(const SimpleIntegerOptionBase& x) = delete;
    SimpleIntegerOptionBase(
        std::string label,
        LockMode lock_while_running,
        NativeType min_value, NativeType max_value,
        NativeType default_value, NativeType current_value
    );


public:
    SimpleIntegerOptionBase(
        std::string label,
        LockMode lock_while_running,
        NativeType default_value
    );
    SimpleIntegerOptionBase(
        std::string label,
        LockMode lock_while_running,
        NativeType default_value, NativeType min_value
    );
    SimpleIntegerOptionBase(
        std::string label,
        LockMode lock_while_running,
        NativeType default_value, NativeType min_value, NativeType max_value
    );

    const std::string& label() const{ return m_label; }


private:
    const std::string m_label;
};



}
#endif
