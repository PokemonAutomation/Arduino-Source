/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_TimeExpressionOptionBase_H
#define PokemonAutomation_TimeExpressionOptionBase_H

#include <QJsonValue>
#include <QLineEdit>

namespace PokemonAutomation{


template <typename Type>
class TimeExpressionOptionBase{
public:
    TimeExpressionOptionBase(
        Type& backing,
        QString label,
        Type min_value,
        Type max_value,
        QString default_value
    );
    TimeExpressionOptionBase(
        QString label,
        Type min_value,
        Type max_value,
        QString default_value
    );

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    operator Type() const{ return m_value; }
    Type value() const{ return m_value; }

    Type min() const{ return m_min_value; }
    Type max() const{ return m_max_value; }

    bool is_valid() const;
    void restore_defaults();
    bool update();

private:
    bool set_value(const QString& str);
    QString time_string() const;

private:
    template <typename> friend class TimeExpressionOptionBaseUI;
    const QString m_label;
    const Type m_min_value;
    const Type m_max_value;
    QString m_default;
    QString m_current;

    Type& m_value;
    Type m_backing;

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
#endif


