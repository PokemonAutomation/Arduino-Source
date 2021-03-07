/*  Switch Date Option
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_SwitchDateOption_H
#define PokemonAutomation_SwitchDateOption_H

#include <QDate>
#include <QJsonValue>
#include <QDateEdit>

namespace PokemonAutomation{


class SwitchDateOption{
public:
    SwitchDateOption(
        QString label,
        QDate default_value
    );
    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    operator QDate() const{ return m_current; }
    QDate value() const{ return m_current; }

    bool is_valid() const;
    void restore_defaults();

protected:
    friend class SwitchDateOptionUI;
    const QString m_label;
    QDate m_default;
    QDate m_current;
};


class SwitchDateOptionUI : public QWidget{
public:
    SwitchDateOptionUI(QWidget& parent, SwitchDateOption& value);
    void restore_defaults();

private:
    SwitchDateOption& m_value;
    QDateEdit* m_date_edit;
};

}
#endif

