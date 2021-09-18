/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FloatingPointOptionBase_H
#define PokemonAutomation_FloatingPointOptionBase_H

#include <QJsonValue>
#include <QLineEdit>

namespace PokemonAutomation{


class FloatingPointOptionBase{
public:
    FloatingPointOptionBase(
        QString label,
        double min_value,
        double max_value,
        double default_value
    );
    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    operator double() const{ return m_current; }
    double value() const{ return m_current; }

    bool is_valid() const;
    void restore_defaults();

protected:
    friend class FloatingPointOptionBaseUI;
    const QString m_label;
    const double m_min_value;
    const double m_max_value;
    double m_default;
    double m_current;
};


class FloatingPointOptionBaseUI : public QWidget{
public:
    FloatingPointOptionBaseUI(QWidget& parent, FloatingPointOptionBase& value);
    void restore_defaults();

private:
    FloatingPointOptionBase& m_value;
    QLineEdit* m_box;
};


}
#endif
