/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_EnumDropdownOption_H
#define PokemonAutomation_EnumDropdownOption_H

#include <atomic>
#include <vector>
#include <map>
#include <QString>
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
    const QString& current_case() const { return m_case_list[m_current]; }

    operator size_t() const{ return m_current.load(std::memory_order_relaxed); }
    void set(size_t index){ m_current.store(index, std::memory_order_relaxed); }

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    QString m_label;
    std::vector<QString> m_case_list;
    std::map<QString, size_t> m_case_map;
    const size_t m_default;
    std::atomic<size_t> m_current;
};




}
#endif
