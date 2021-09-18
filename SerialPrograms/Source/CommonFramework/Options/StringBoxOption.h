/*  String Box Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StringBoxOption_H
#define PokemonAutomation_StringBoxOption_H

#include "ConfigOption.h"

namespace PokemonAutomation{


class StringBoxOption : public ConfigOption{
public:
    StringBoxOption(
        QString& backing,
        QString label,
        QString default_value
    );
    StringBoxOption(
        QString label,
        QString default_value
    );


    virtual void load_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

    operator QString() const{ return m_current; }
    QString value() const{ return m_current; }

    void restore_defaults();

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class StringBoxOptionUI;
    const QString m_label;
    QString m_default;
    QString& m_current;
    QString m_backing;
};


class StringBoxOptionUI : public ConfigOptionUI, public QWidget{
public:
    StringBoxOptionUI(QWidget& parent, StringBoxOption& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override;

private:
    class Box;

    StringBoxOption& m_value;
    Box* m_box;
};



inline ConfigOptionUI* StringBoxOption::make_ui(QWidget& parent){
    return new StringBoxOptionUI(parent, *this);
}



}
#endif
