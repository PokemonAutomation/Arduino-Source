/*  Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_DateOption_H
#define PokemonAutomation_Options_DateOption_H

#include <QDate>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Options/ConfigOption.h"

namespace PokemonAutomation{


class DateOption : public ConfigOption{
public:
    DateOption(
        std::string label,
        QDate min_value, QDate max_value,
        QDate default_value
    );
//    virtual std::unique_ptr<ConfigOption> clone() const override;

    const std::string& label() const{ return m_label; }
    const QDate& min_value() const{ return m_min_value; }
    const QDate& max_value() const{ return m_max_value; }
    const QDate& default_value() const{ return m_default; }

    operator QDate() const;
    QDate get() const;
    std::string set(QDate x);

    static QDate from_json(const JsonValue& json);
    static JsonValue to_json(const QDate& date);

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    std::string check_validity(QDate x) const;
    virtual std::string check_validity() const override;
    virtual void restore_defaults() override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    const std::string m_label;
    const QDate m_min_value;
    const QDate m_max_value;
    const QDate m_default;

    mutable SpinLock m_lock;
    QDate m_current;
};




}
#endif

