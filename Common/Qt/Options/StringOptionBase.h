/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StringOptionBase_H
#define PokemonAutomation_StringOptionBase_H

#include <QJsonValue>
#include <QLineEdit>
#include <QString>
#include <QWidget>

namespace PokemonAutomation{


class StringOptionBase{
public:
    StringOptionBase(
        QString& backing,
        QString label,
        QString default_value
    );
    StringOptionBase(
        QString label,
        QString default_value
    );

    void load_default(const QJsonValue& json);
    void load_current(const QJsonValue& json);
    QJsonValue write_default() const;
    QJsonValue write_current() const;

    operator QString() const{ return m_current; }
    QString value() const{ return m_current; }

    void restore_defaults();

private:
    friend class StringOptionBaseUI;
    const QString m_label;
    QString m_default;
    QString& m_current;
    QString m_backing;
};


class StringOptionBaseUI : public QWidget{
public:
    StringOptionBaseUI(QWidget& parent, StringOptionBase& value);
    void restore_defaults();

private:
    StringOptionBase& m_value;
    QLineEdit * m_line_edit;
};

}
#endif
