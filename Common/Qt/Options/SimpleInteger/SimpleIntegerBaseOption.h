/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SimpleIntegerBaseOption_H
#define PokemonAutomation_SimpleIntegerBaseOption_H

#include <atomic>
#include <QString>

class QJsonValue;

namespace PokemonAutomation{

class JsonValue2;


template <typename Type>
class SimpleIntegerBaseOption{
public:
    SimpleIntegerBaseOption(
        QString label,
        Type min_value,
        Type max_value,
        Type default_value
    );

    const QString& label() const{ return m_label; }

    operator Type() const{ return m_current.load(std::memory_order_relaxed); }
    Type get() const{ return m_current.load(std::memory_order_relaxed); }
    QString set(Type x);

    QString check_validity() const;
    QString check_validity(Type x) const;
    void restore_defaults();

    void load_default(const JsonValue2& json);
    void load_current(const JsonValue2& json);
    JsonValue2 write_default() const;
    JsonValue2 write_current() const;


protected:
    const QString m_label;
    const Type m_min_value;
    const Type m_max_value;
    Type m_default;
    std::atomic<Type> m_current;
};




}
#endif
