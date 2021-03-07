/*  Boolean Check Box Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_BooleanCheckBoxOption_H
#define PokemonAutomation_BooleanCheckBoxOption_H

#include <QJsonValue>
#include <QCheckBox>

namespace PokemonAutomation{


class BooleanCheckBoxOption{
public:
    BooleanCheckBoxOption(
        bool& backing,
        QString label,
        bool default_value
    );
    BooleanCheckBoxOption(
        QString label,
        bool default_value
    );

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    operator bool() const{ return m_current; }
    bool value() const{ return m_current; }

    void restore_defaults();

private:
    friend class BooleanCheckBoxOptionUI;
    const QString m_label;
    bool m_default;
    bool& m_current;
    bool m_backing;
};


class BooleanCheckBoxOptionUI : public QWidget{
public:
    BooleanCheckBoxOptionUI(QWidget& parent, BooleanCheckBoxOption& value);
    void restore_defaults();

private:
    BooleanCheckBoxOption& m_value;
    QCheckBox* m_box;
};

}
#endif
