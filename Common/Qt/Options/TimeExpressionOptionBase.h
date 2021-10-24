/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This option is thread-safe.
 *
 */

#ifndef PokemonAutomation_TimeExpressionOptionBase_H
#define PokemonAutomation_TimeExpressionOptionBase_H

#include <QJsonValue>
#include <QLineEdit>
#include "Common/Cpp/SpinLock.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


template <typename Type>
class TimeExpressionOptionBase{
public:
    TimeExpressionOptionBase(
        QString label,
        Type min_value,
        Type max_value,
        QString default_value
    );

    const QString& label() const{ return m_label; }

    operator Type() const;
    Type get() const;
    QString set(QString text);

    QString text() const;
    QString time_string() const;

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    Type min() const{ return m_min_value; }
    Type max() const{ return m_max_value; }

    QString check_validity() const;
    void restore_defaults();

private:
    QString process(const QString& text, Type& value) const;

private:
    const QString m_label;
    const Type m_min_value;
    const Type m_max_value;
    QString m_default;

    mutable SpinLock m_lock;
    QString m_current;
    Type m_value;
    QString m_error;
};


template <typename Type>
class TimeExpressionOptionBaseUI : public QWidget{
public:
    TimeExpressionOptionBaseUI(QWidget& parent, TimeExpressionOptionBase<Type>& value);
    void restore_defaults();

private:
    TimeExpressionOptionBase<Type>& m_value;
    QLineEdit* m_box;
};


}
}
#endif


