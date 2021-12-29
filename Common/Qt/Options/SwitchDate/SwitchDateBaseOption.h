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

class QJsonValue;

namespace PokemonAutomation{
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

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;


protected:
    const QString m_label;
    QDate m_default;

    mutable SpinLock m_lock;
    QDate m_current;
};



}
}
#endif

