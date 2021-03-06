/*  Time Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_TimeExpressionOption_H
#define PokemonAutomation_TimeExpressionOption_H

#include <QJsonValue>
#include <QLineEdit>

namespace PokemonAutomation{


template <typename Type>
class TimeExpressionOption{
public:
    TimeExpressionOption(
        Type& backing,
        QString label,
        Type min_value,
        Type max_value,
        QString default_value
    );
    TimeExpressionOption(
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
    template <typename> friend class TimeExpressionOptionUI;
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
class TimeExpressionOptionUI : public QWidget{
public:
    TimeExpressionOptionUI(QWidget& parent, TimeExpressionOption<Type>& value);
    void restore_defaults();

private:
    TimeExpressionOption<Type>& m_value;
    QLineEdit* m_box;
};


}
#endif


