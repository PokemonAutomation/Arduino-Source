/*  Enum Dropdown
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EnumDropdown_H
#define PokemonAutomation_EnumDropdown_H

#include <vector>
#include <map>
#include "SingleStatementOption.h"

namespace PokemonAutomation{


class EnumDropdown : public SingleStatementOption{
public:
    static const std::string OPTION_TYPE;
    static const std::string JSON_OPTIONS;

public:
    EnumDropdown(const std::string& declaration, const std::string& label, size_t default_value);
    EnumDropdown(const JsonObject& obj);

    virtual const std::string& type() const override{ return OPTION_TYPE; }
    virtual void restore_defaults() override;

    virtual JsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class EnumDropdownUI;
    std::vector<std::pair<std::string, std::string>> m_options;
    std::map<std::string, size_t> m_map;
    size_t m_default;
    size_t m_current;
};



class EnumDropdownUI : public QWidget{
public:
    EnumDropdownUI(QWidget& parent, EnumDropdown& value, const std::string& label);
    ~EnumDropdownUI();
private:
    EnumDropdown& m_value;
};



}
#endif


