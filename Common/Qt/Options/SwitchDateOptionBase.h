/*  Switch Date Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SwitchDateOptionBase_H
#define PokemonAutomation_SwitchDateOptionBase_H

#include <QDate>
#include <QJsonValue>
#include <QDateEdit>

namespace PokemonAutomation{


class SwitchDateOptionBase{
public:
    SwitchDateOptionBase(
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
    friend class SwitchDateOptionBaseUI;
    const QString m_label;
    QDate m_default;
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
#endif

