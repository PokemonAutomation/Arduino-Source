/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EnumDropdown_H
#define PokemonAutomation_EnumDropdown_H

#include <map>
#include <QComboBox>
#include "ConfigOption.h"

namespace PokemonAutomation{


class EnumDropdownOption : public ConfigOption{
public:
    EnumDropdownOption(
        QString label,
        std::vector<QString> cases,
        size_t default_index
    );

    operator size_t() const{ return m_current; }

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class EnumDropdownOptionUI;
    QString m_label;
    std::vector<QString> m_case_list;
    std::map<QString, size_t> m_case_map;
    size_t m_default;
    size_t m_current;
};



class EnumDropdownOptionUI : public ConfigOptionUI, public QWidget{
public:
    EnumDropdownOptionUI(QWidget& parent, EnumDropdownOption& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    EnumDropdownOption& m_value;
    QComboBox* m_box;
};



}
#endif
