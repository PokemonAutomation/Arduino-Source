/*  Config Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ConfigOption_H
#define PokemonAutomation_ConfigOption_H

#include <QString>
#include <QJsonValue>
#include <QWidget>

class ConfigOptionUI;

class ConfigOption{
public:
    ConfigOption(QString label)
        : m_label(std::move(label))
    {}
    virtual void load_json(const QJsonValue& json) = 0;
    virtual QJsonValue to_json() const = 0;

    virtual bool is_valid() const{ return true; };
    virtual void restore_defaults(){};

    virtual ConfigOptionUI* make_ui(QWidget& parent) = 0;

protected:
    QString m_label;
};


class ConfigOptionUI{
public:
    virtual ~ConfigOptionUI() = default;
    virtual QWidget* widget() = 0;
    virtual void restore_defaults() = 0;
};



#endif
