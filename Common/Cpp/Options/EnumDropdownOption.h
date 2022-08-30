/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A simple dropdown menu for enums.
 *
 *  This should only be used when the number of options is small. If you need
 *  something for many options, use StringSelectCell/Option instead.
 *
 */

#ifndef PokemonAutomation_Options_EnumDropdownOption_H
#define PokemonAutomation_Options_EnumDropdownOption_H

#include "Common/Cpp/EnumDatabase.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


//  This is the typeless class that uses an integer for the enum value.
class IntegerEnumDropdownCell : public ConfigOption{
public:
    ~IntegerEnumDropdownCell();
    IntegerEnumDropdownCell(const IntegerEnumDropdownCell& x);

public:
    IntegerEnumDropdownCell(const IntegerEnumDatabase& database, size_t default_value, size_t current_value);
    IntegerEnumDropdownCell(const IntegerEnumDatabase& database, size_t default_value);

    //  Constructing from inlined database is not supported for cell.
    IntegerEnumDropdownCell(IntegerEnumDatabase&& database, size_t default_value, size_t current_value) = delete;
    IntegerEnumDropdownCell(IntegerEnumDatabase&& database, size_t default_value) = delete;

    size_t default_value() const;
    size_t current_value() const;

    virtual bool set_value(size_t value);

    const IntegerEnumDatabase& database() const;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    struct Data;
    Pimpl<Data> m_data;
};


//  This is the type-safe class for your enum type.
template <typename EnumType>
class EnumDropdownCell : public IntegerEnumDropdownCell{
public:
    EnumDropdownCell(const EnumDatabase<EnumType>& database, EnumType default_value)
        : IntegerEnumDropdownCell(database, (size_t)default_value)
    {}

    //  Constructing from inlined database is not supported for cell.
    EnumDropdownCell(EnumDatabase<EnumType>&& database, EnumType default_value) = delete;

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




class IntegerEnumDropdownOption : private IntegerEnumDatabase, public IntegerEnumDropdownCell{
public:
    IntegerEnumDropdownOption(const IntegerEnumDropdownOption& x) = delete;
    IntegerEnumDropdownOption(
        std::string label,
        const IntegerEnumDatabase& database,
        size_t default_value
    )
        : IntegerEnumDatabase(nullptr)
        , IntegerEnumDropdownCell(database, default_value)
        , m_label(std::move(label))
    {}
    IntegerEnumDropdownOption(
        std::string label,
        IntegerEnumDatabase&& database,
        size_t default_value
    )
        : IntegerEnumDatabase(std::move(database))
        , IntegerEnumDropdownCell(*this, default_value)
        , m_label(std::move(label))
    {}

    const std::string& label() const{ return m_label; }
    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
};



template <typename EnumType>
class EnumDropdownOption : public IntegerEnumDropdownOption{
public:
    EnumDropdownOption(
        std::string label,
        const EnumDatabase<EnumType>& database,
        EnumType default_value
    )
        : IntegerEnumDropdownOption(std::move(label), database, (size_t)default_value)
    {}
    EnumDropdownOption(
        std::string label,
        EnumDatabase<EnumType>&& database,
        EnumType default_value
    )
        : IntegerEnumDropdownOption(std::move(label), std::move(database), (size_t)default_value)
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
