/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_EnumDropdownOption_H
#define PokemonAutomation_EnumDropdownOption_H

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <QString>
#include "ConfigOption.h"

namespace PokemonAutomation{


class EnumDropdownOption : public ConfigOption{
public:
    struct Option{
        std::string name;
        bool enabled = true;
        Option(const char* p_name)
            : name(p_name)
        {}
        Option(std::string p_name, bool p_enabled = true)
            : name(std::move(p_name))
            , enabled(p_enabled)
        {}
    };

public:
    template <typename ListType>
    EnumDropdownOption(
        std::string label,
        ListType&& cases,
        size_t default_index
    );
    EnumDropdownOption(
        std::string label,
        std::initializer_list<Option> cases,
        size_t default_index
    );

    const std::string& label() const{ return m_label; }
    const std::string& case_name(size_t index) const{ return m_case_list[index].name; }
    const std::string& current_case() const { return m_case_list[m_current].name; }

    operator size_t() const{ return m_current.load(std::memory_order_relaxed); }
    void set(size_t index){ m_current.store(index, std::memory_order_relaxed); }

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class EnumDropdownWidget;

    std::string m_label;
    std::vector<Option> m_case_list;
    std::map<std::string, size_t> m_case_map;
    const size_t m_default;
    std::atomic<size_t> m_current;
};








template <typename ListType>
EnumDropdownOption::EnumDropdownOption(
    std::string label,
    ListType&& cases,
    size_t default_index
)
    : m_label(std::move(label))
    , m_default(default_index)
    , m_current(default_index)
{
    if (default_index >= cases.size()){
        throw "Index is too large.";
    }
    for (size_t index = 0; index < cases.size(); index++){
        m_case_list.emplace_back(std::move(cases[index]));
        const std::string& item = m_case_list.back().name;
        auto ret = m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item),
            std::forward_as_tuple(index)
        );
        if (!ret.second){
            throw "Duplicate enum label.";
        }
    }
}





}
#endif
