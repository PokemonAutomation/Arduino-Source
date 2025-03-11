/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A simple dropdown menu for enums.
 *
 *  This should only be used when the number of options is small. If you need
 *  something for many options, use StringSelectCell/Option instead.
 *
 */

#ifndef PokemonAutomation_Options_EnumDropdownOption_H
#define PokemonAutomation_Options_EnumDropdownOption_H

#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"
#include "EnumDropdownDatabase.h"

namespace PokemonAutomation{


//  This is the typeless class that uses an integer for the enum value.
class IntegerEnumDropdownCell : public ConfigOption{
public:
    ~IntegerEnumDropdownCell();
    IntegerEnumDropdownCell(const IntegerEnumDropdownCell& x);

public:
    IntegerEnumDropdownCell(
        const IntegerEnumDropdownDatabase& database,
        LockMode lock_while_running,
        size_t default_value, size_t current_value
    );
    IntegerEnumDropdownCell(
        const IntegerEnumDropdownDatabase& database,
        LockMode lock_while_running,
        size_t default_value
    );

    //  Constructing from inlined database is not supported for cell.
    IntegerEnumDropdownCell(IntegerEnumDropdownDatabase&& database,
                            LockMode lock_while_running,
        size_t default_value, size_t current_value
    ) = delete;
    IntegerEnumDropdownCell(IntegerEnumDropdownDatabase&& database,
                            LockMode lock_while_running,
        size_t default_value
    ) = delete;

    size_t default_value() const;
    size_t current_value() const;

    virtual bool set_value(size_t value);

    const IntegerEnumDropdownDatabase& database() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};


//  This is the type-safe class for your enum type.
template <typename EnumType>
class EnumDropdownCell : public IntegerEnumDropdownCell{
public:
    EnumDropdownCell(
        const EnumDropdownDatabase<EnumType>& database,
        LockMode lock_while_running,
        EnumType default_value
    )
        : IntegerEnumDropdownCell(database, lock_while_running, (size_t)default_value)
    {}

    //  Constructing from inlined database is not supported for cell.
    EnumDropdownCell(
        EnumDropdownDatabase<EnumType>&& database,
        LockMode lock_while_running,
        EnumType default_value
    ) = delete;

    operator EnumType() const{
        return (EnumType)current_value();
    }
    bool set(EnumType value){
        return set_value((size_t)value);
    }

#if 1
    const std::string& current_slug() const{
        return database().find(current_value())->slug;
    }
    const std::string& current_display() const{
        return database().find(current_value())->display;
    }
#endif
};




class IntegerEnumDropdownOption : private IntegerEnumDropdownDatabase, public IntegerEnumDropdownCell{
public:
    IntegerEnumDropdownOption(const IntegerEnumDropdownOption& x) = delete;
    IntegerEnumDropdownOption(
        std::string label,
        const IntegerEnumDropdownDatabase& database,
        LockMode lock_while_running,
        size_t default_value
    )
        : IntegerEnumDropdownDatabase(nullptr)
        , IntegerEnumDropdownCell(database, lock_while_running, default_value)
        , m_label(std::move(label))
    {}
    IntegerEnumDropdownOption(
        std::string label,
        IntegerEnumDropdownDatabase&& database,
        LockMode lock_while_running,
        size_t default_value
    )
        : IntegerEnumDropdownDatabase(std::move(database))
        , IntegerEnumDropdownCell(*this, lock_while_running, default_value)
        , m_label(std::move(label))
    {}

    const std::string& label() const{ return m_label; }
    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

private:
    const std::string m_label;
};



template <typename EnumType>
class EnumDropdownOption : public IntegerEnumDropdownOption{
public:
    EnumDropdownOption(
        std::string label,
        const EnumDropdownDatabase<EnumType>& database,
        LockMode lock_while_running,
        EnumType default_value
    )
        : IntegerEnumDropdownOption(std::move(label), database, lock_while_running, (size_t)default_value)
    {}
    EnumDropdownOption(
        std::string label,
        EnumDropdownDatabase<EnumType>&& database,
        LockMode lock_while_running,
        EnumType default_value
    )
        : IntegerEnumDropdownOption(std::move(label), std::move(database), lock_while_running, (size_t)default_value)
    {}

    operator EnumType() const{
        return (EnumType)current_value();
    }
    EnumType get() const{
        return (EnumType)current_value();
    }
    bool set(EnumType value){
        return set_value((size_t)value);
    }
};






}
#endif
