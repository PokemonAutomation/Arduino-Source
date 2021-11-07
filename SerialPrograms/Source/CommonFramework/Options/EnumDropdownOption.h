/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
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

    const QString& label() const{ return m_label; }
    const QString& case_name(size_t index) const{ return m_case_list[index]; }
    const std::vector<QString>& case_list() const{ return m_case_list; }

    operator size_t() const{ return m_current.load(std::memory_order_relaxed); }
    void set(size_t index){ m_current.store(index, std::memory_order_relaxed); }

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    QString m_label;
    std::vector<QString> m_case_list;
    std::map<QString, size_t> m_case_map;
    const size_t m_default;
    std::atomic<size_t> m_current;
};



class EnumDropdownOptionUI : public QWidget, public ConfigOptionUI{
    Q_OBJECT

public:
    EnumDropdownOptionUI(QWidget& parent, EnumDropdownOption& value);
    virtual void restore_defaults() override;

signals:
    void on_changed();

private:
    EnumDropdownOption& m_value;
    QComboBox* m_box;
};



}
#endif
