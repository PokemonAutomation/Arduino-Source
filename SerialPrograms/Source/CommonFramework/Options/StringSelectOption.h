/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_StringSelect_H
#define PokemonAutomation_StringSelect_H

#include <vector>
#include <map>
#include <QString>
#include <QIcon>
#include "ConfigOption.h"

namespace PokemonAutomation{


class StringSelectOption : public ConfigOption{
public:
    StringSelectOption(
        QString label,
        const std::vector<QString>& cases,
        const QString& default_case
    );
    StringSelectOption(
        QString label,
        std::vector<std::pair<QString, QIcon>> cases,
        const QString& default_case
    );

    operator size_t() const{ return m_current.load(std::memory_order_relaxed); }
    operator const QString&() const{ return m_case_list[(size_t)*this].first; }

    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class StringSelectWidget;
    const QString m_label;
    std::vector<std::pair<QString, QIcon>> m_case_list;
    std::map<QString, size_t> m_case_map;
    size_t m_default;

    std::atomic<size_t> m_current;
};





}
#endif
