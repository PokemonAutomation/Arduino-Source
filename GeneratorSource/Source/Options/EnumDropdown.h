/*  Enum Dropdown
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
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
    static const QString OPTION_TYPE;
    static const QString JSON_OPTIONS;

public:
    EnumDropdown(const QString& declaration, const QString& label, size_t default_value);
    EnumDropdown(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual void restore_defaults() override;

    virtual QJsonObject to_json() const override;
    virtual std::string to_cpp() const override;

    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class EnumDropdownUI;
    std::vector<std::pair<QString, QString>> m_options;
    std::map<QString, size_t> m_map;
    size_t m_default;
    size_t m_current;
};



class EnumDropdownUI : public QWidget{
public:
    EnumDropdownUI(QWidget& parent, EnumDropdown& value, const QString& label);
    ~EnumDropdownUI();
private:
    EnumDropdown& m_value;
};



}
#endif


