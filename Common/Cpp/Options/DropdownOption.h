/*  Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A simple dropdown menu for strings.
 *
 *  This should only be used when the number of options is small. If you need
 *  something for many options, use StringSelectCell/Option instead.
 *
 */

#ifndef PokemonAutomation_Options_DropdownOption_H
#define PokemonAutomation_Options_DropdownOption_H

#include <vector>
#include "Common/Compiler.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "ConfigOption.h"

namespace PokemonAutomation{



struct DropdownEntry{
    std::string slug;
    std::string display_name;
    bool enabled = true;
    DropdownEntry(std::string p_slug, std::string p_display_name, bool p_enabled)
        : slug(std::move(p_slug))
        , display_name(std::move(p_display_name))
        , enabled(p_enabled)
    {}
    DropdownEntry(std::string p_slug, std::string p_display_name)
        : slug(std::move(p_slug))
        , display_name(std::move(p_display_name))
        , enabled(true)
    {}
    DropdownEntry(std::string p_display_name)
        : slug(p_display_name)
        , display_name(std::move(p_display_name))
        , enabled(true)
    {}
    DropdownEntry(const char* p_display_name)
        : slug(p_display_name)
        , display_name(p_display_name)
    {}
};


class DropdownCell : public ConfigOption{
public:
    ~DropdownCell();
    DropdownCell(const DropdownCell& x);

public:
    DropdownCell(
        std::vector<std::string> cases,
        size_t default_index
    );
    DropdownCell(
        std::vector<DropdownEntry> cases,
        size_t default_index
    );
    DropdownCell(
        std::initializer_list<DropdownEntry> cases,
        size_t default_index
    );

    const std::vector<DropdownEntry>& case_list() const;
    size_t default_index() const;
    size_t current_index() const;

    const std::string& case_name(size_t index) const;
    const std::string& current_case() const;

    operator size_t() const;
    virtual bool set_index(size_t index);   //  Returns false if index is out of range.

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

protected:
    struct Data;
    Pimpl<Data> m_data;
};






}
#endif
