/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_Options_SimpleIntegerOption_H
#define PokemonAutomation_Options_SimpleIntegerOption_H

#include "SimpleIntegerOptionBase.h"

namespace PokemonAutomation{



template <typename Type>
class SimpleIntegerCell : public SimpleIntegerCellBase{
public:
    SimpleIntegerCell(const SimpleIntegerCell& x)
        : SimpleIntegerCellBase(x)
    {}
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type min_value, Type max_value,
        Type default_value, Type current_value
    )
        : SimpleIntegerCellBase(
            lock_while_running,
            sanitize(min_value),
            sanitize(max_value),
            sanitize(default_value),
            sanitize(current_value)
        )
    {}


public:
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type default_value
    )
        : SimpleIntegerCellBase(
            lock_while_running,
            sanitize(default_value)
        )
    {}
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type default_value, Type min_value
    )
        : SimpleIntegerCellBase(
            lock_while_running,
            sanitize(default_value),
            sanitize(min_value)
        )
    {}
    SimpleIntegerCell(
        LockMode lock_while_running,
        Type default_value, Type min_value, Type max_value
    )
        : SimpleIntegerCellBase(
            lock_while_running,
            sanitize(default_value),
            sanitize(min_value),
            sanitize(max_value)
        )
    {}

    Type min_value() const{
        return (Type)SimpleIntegerCellBase::min_value();
    }
    Type max_value() const{
        return (Type)SimpleIntegerCellBase::max_value();
    }
    Type default_value() const{
        return (Type)SimpleIntegerCellBase::default_value();
    }
    Type current_value() const{
        return (Type)SimpleIntegerCellBase::current_value();
    }

    operator Type() const{
        return (Type)SimpleIntegerCellBase::current_value();
    }
    std::string set(Type x){
        return SimpleIntegerCellBase::set(x);
    }

    using SimpleIntegerCellBase::load_json;
    using SimpleIntegerCellBase::to_json;
    std::string check_validity(Type x) const{
        return SimpleIntegerCellBase::check_validity(x);
    }
    using SimpleIntegerCellBase::check_validity;
    using SimpleIntegerCellBase::restore_defaults;
};





template <typename Type>
class SimpleIntegerOption : public SimpleIntegerOptionBase{
public:
    SimpleIntegerOption(const SimpleIntegerOption& x) = delete;
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type min_value, Type max_value,
        Type default_value, Type current_value
    )
        : SimpleIntegerOptionBase(
            std::move(label),
            lock_while_running,
            sanitize(min_value),
            sanitize(max_value),
            sanitize(default_value),
            sanitize(current_value)
        )
    {}


public:
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type default_value
    )
        : SimpleIntegerOptionBase(
            std::move(label),
            lock_while_running,
            sanitize(default_value)
        )
    {}
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type default_value, Type min_value
    )
        : SimpleIntegerOptionBase(
            std::move(label),
            lock_while_running,
            sanitize(default_value),
            sanitize(min_value)
        )
    {}
    SimpleIntegerOption(
        std::string label,
        LockMode lock_while_running,
        Type default_value, Type min_value, Type max_value
    )
        : SimpleIntegerOptionBase(
            std::move(label),
            lock_while_running,
            sanitize(default_value),
            sanitize(min_value),
            sanitize(max_value)
        )
    {}

    using SimpleIntegerOptionBase::label;

    Type min_value() const{
        return (Type)SimpleIntegerCellBase::min_value();
    }
    Type max_value() const{
        return (Type)SimpleIntegerCellBase::max_value();
    }
    Type default_value() const{
        return (Type)SimpleIntegerCellBase::default_value();
    }
    Type current_value() const{
        return (Type)SimpleIntegerCellBase::current_value();
    }

    operator Type() const{
        return (Type)SimpleIntegerCellBase::current_value();
    }
    std::string set(Type x){
        return SimpleIntegerCellBase::set(x);
    }

    using SimpleIntegerCellBase::load_json;
    using SimpleIntegerCellBase::to_json;
    std::string check_validity(Type x) const{
        return SimpleIntegerCellBase::check_validity(x);
    }
    using SimpleIntegerCellBase::check_validity;
    using SimpleIntegerCellBase::restore_defaults;
};










}
#endif

