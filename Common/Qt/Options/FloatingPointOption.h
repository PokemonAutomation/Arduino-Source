/*  Floating-Point Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_FloatingPointOption_H
#define PokemonAutomation_FloatingPointOption_H

#include <QJsonValue>
#include <QLineEdit>

namespace PokemonAutomation{


class FloatingPointOption{
public:
    FloatingPointOption(
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
    friend class FloatingPointOptionUI;
    const QString m_label;
    const double m_min_value;
    const double m_max_value;
    double m_default;
    double m_current;
};


class FloatingPointOptionUI : public QWidget{
public:
    FloatingPointOptionUI(QWidget& parent, FloatingPointOption& value);
    void restore_defaults();

private:
    FloatingPointOption& m_value;
    QLineEdit* m_box;
};


}
#endif
