/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SwitchDateBaseOption_H
#define PokemonAutomation_SwitchDateBaseOption_H

#include <QDate>
#include "Common/Cpp/SpinLock.h"

namespace PokemonAutomation{
    class JsonValue2;
namespace NintendoSwitch{


class SwitchDateBaseOption{
public:
    SwitchDateBaseOption(
        QString label,
        QDate default_value
    );

    const QString& label() const{ return m_label; }

    operator QDate() const;
    QDate get() const;
    QString set(QDate x);

    QString check_validity() const;
    QString check_validity(QDate x) const;
    void restore_defaults();

    void load_default(const JsonValue2& json);
    void load_current(const JsonValue2& json);
    JsonValue2 write_default() const;
    JsonValue2 write_current() const;


protected:
    const QString m_label;
    QDate m_default;

    mutable SpinLock m_lock;
    QDate m_current;
};



}
}
#endif

