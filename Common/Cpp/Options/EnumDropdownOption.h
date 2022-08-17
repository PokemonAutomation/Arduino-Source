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

#include <vector>
#include "Common/Compiler.h"
#include "Common/Cpp/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{



struct EnumDropdownEntry{
    std::string name;
    bool enabled = true;
    EnumDropdownEntry(const char* p_name)
        : name(p_name)
    {}
    EnumDropdownEntry(std::string p_name, bool p_enabled = true)
        : name(std::move(p_name))
        , enabled(p_enabled)
    {}
};


class EnumDropdownCell : public ConfigOption{
public:
    ~EnumDropdownCell();
    EnumDropdownCell(const EnumDropdownCell& x);

public:
    EnumDropdownCell(
        std::vector<std::string> cases,
        size_t default_index
    );
    EnumDropdownCell(
        std::vector<EnumDropdownEntry> cases,
        size_t default_index
    );
    EnumDropdownCell(
        std::initializer_list<EnumDropdownEntry> cases,
        size_t default_index
    );

    const std::vector<EnumDropdownEntry>& case_list() const;
    size_t default_index() const;

    const std::string& case_name(size_t index) const;
    const std::string& current_case() const;

    operator size_t() const;
    virtual bool set(size_t index); //  Returns false if index is out of range.

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

protected:
    struct Data;
    Pimpl<Data> m_data;
};



class EnumDropdownOption : public EnumDropdownCell{
public:
    EnumDropdownOption(const EnumDropdownOption& x) = delete;
    EnumDropdownOption(
        std::string label,
        std::vector<std::string> cases,
        size_t default_index
    )
        : EnumDropdownCell(cases, default_index)
        , m_label(std::move(label))
    {}
    EnumDropdownOption(
        std::string label,
        std::vector<EnumDropdownEntry> cases,
        size_t default_index
    )
        : EnumDropdownCell(cases, default_index)
        , m_label(std::move(label))
    {}
    EnumDropdownOption(
        std::string label,
        std::initializer_list<EnumDropdownEntry> cases,
        size_t default_index
    )
        : EnumDropdownCell(std::move(cases), default_index)
        , m_label(std::move(label))
    {}

    const std::string& label() const{ return m_label; }
    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
};








}
#endif
