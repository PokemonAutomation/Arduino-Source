/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_SwitchDateOptionBase_H
#define PokemonAutomation_SwitchDateOptionBase_H

#include <QDate>
#include <QJsonValue>
#include <QDateEdit>
#include "Common/Cpp/SpinLock.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchDateOptionBase{
public:
    SwitchDateOptionBase(
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


class SwitchDateOptionBaseUI : public QWidget{
public:
    SwitchDateOptionBaseUI(QWidget& parent, SwitchDateOptionBase& value);
    void restore_defaults();

private:
    SwitchDateOptionBase& m_value;
    QDateEdit* m_date_edit;
};



}
}
#endif

