/*  Checkbox Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Same as EnumDropdown, but allows multiple selections.
 *
 */

#ifndef PokemonAutomation_Options_CheckboxDropdownOption_H
#define PokemonAutomation_Options_CheckboxDropdownOption_H

#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ConfigOption.h"

namespace PokemonAutomation{


template <typename FlagEnum>
class CheckboxDropdownDatabase;


class CheckboxDropdownBase : public ConfigOptionImpl<CheckboxDropdownBase>{
public:
    CheckboxDropdownBase(std::string label)
        : m_label(std::move(label))
    {}

    virtual std::string current_label() const = 0;
    virtual size_t size() const = 0;
    virtual const std::string& name_at_index(size_t index) const = 0;

    virtual bool operator[](size_t index) const = 0;
    virtual void set_index(size_t index) = 0;
    virtual void clear_index(size_t index) = 0;
    virtual void toggle_index(size_t index) = 0;


protected:
    std::string m_label;
};



template <typename FlagEnum>
class CheckboxDropdownCell : public CheckboxDropdownBase{
    using Database = CheckboxDropdownDatabase<FlagEnum>;

public:
    //  If "label" is not empty, it will always display that on the dropdown.
    //  If "label" is empty, it will display the checked items instead.
    CheckboxDropdownCell(
        std::string label,
        const Database& database,
        LockMode lock_while_running,
        FlagEnum default_value
    );
    CheckboxDropdownCell(
        std::string label,
        const Database& database,
        LockMode lock_while_running,
        FlagEnum default_value, FlagEnum current_value
    );
    CheckboxDropdownCell(
        std::string label,
        Database&& database,
        LockMode lock_while_running,
        FlagEnum default_value
    ) = delete;
    CheckboxDropdownCell(
        std::string label,
        Database&& database,
        LockMode lock_while_running,
        FlagEnum default_value, FlagEnum current_value
    ) = delete;

    FlagEnum default_value() const{
        return m_default;
    }
    FlagEnum current_value() const;
    operator FlagEnum() const{
        return current_value();
    }


public:
    bool is_set(FlagEnum value) const;

    void replace_all(FlagEnum value);
    void clear();

    void set_flag(FlagEnum value);
    void clear_flag(FlagEnum value);
    void toggle_flag(FlagEnum value);


public:
    virtual std::string current_label() const override;
    virtual size_t size() const override;
    virtual const std::string& name_at_index(size_t index) const override;

    virtual bool operator[](size_t index) const override;
    virtual void set_index(size_t index) override;
    virtual void clear_index(size_t index) override;
    virtual void toggle_index(size_t index) override;


public:
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;


private:
    const Database& m_database;
    const FlagEnum m_default;

    mutable SpinLock m_lock;
    FlagEnum m_current;
};




}
#endif
