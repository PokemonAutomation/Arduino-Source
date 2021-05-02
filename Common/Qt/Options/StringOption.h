/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StringOption_H
#define PokemonAutomation_StringOption_H

#include <QJSonValue>
#include <QLineEdit>
#include <QString>
#include <QWidget>

namespace PokemonAutomation{


class StringOption{
public:
    StringOption(
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
    friend class StringOptionUI;
    const QString m_label;
    QString m_default;
    QString m_current;
};


class StringOptionUI : public QWidget{
public:
    StringOptionUI(QWidget& parent, StringOption& value);
    void restore_defaults();

private:
    StringOption& m_value;
    QLineEdit * m_line_edit;
};

}
#endif
