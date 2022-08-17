/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_Options_EnumDropdownOption_H
#define PokemonAutomation_Options_EnumDropdownOption_H

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "ConfigOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class EnumDropdownCell : public ConfigOption{
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
    EnumDropdownCell(
        ListType&& cases,
        size_t default_index
    );
    EnumDropdownCell(
        std::initializer_list<Option> cases,
        size_t default_index
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::vector<Option>& case_list() const{ return m_case_list; }

    const std::string& case_name(size_t index) const{ return m_case_list[index].name; }
    const std::string& current_case() const { return m_case_list[m_current].name; }

    operator size_t() const{ return m_current.load(std::memory_order_relaxed); }
    virtual bool set(size_t index); //  Returns false if index is out of range.

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

protected:
    std::vector<Option> m_case_list;
    std::map<std::string, size_t> m_case_map;
    const size_t m_default;
    std::atomic<size_t> m_current;
};



class EnumDropdownOption : public EnumDropdownCell{
public:
    template <typename ListType>
    EnumDropdownOption(
        std::string label,
        ListType&& cases,
        size_t default_index
    )
        : EnumDropdownCell(std::move(cases), default_index)
        , m_label(std::move(label))
    {}
    EnumDropdownOption(
        std::string label,
        std::initializer_list<Option> cases,
        size_t default_index
    )
        : EnumDropdownCell(std::move(cases), default_index)
        , m_label(std::move(label))
    {}
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const{ return m_label; }
    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
};







template <typename ListType>
EnumDropdownCell::EnumDropdownCell(
    ListType&& cases,
    size_t default_index
)
    : m_default(default_index)
    , m_current(default_index)
{
//    cout << default_index << endl;
    if (default_index >= cases.size()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Index is too large: " + std::to_string(default_index));
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
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate enum label: " + item);
        }
    }
}








}
#endif
