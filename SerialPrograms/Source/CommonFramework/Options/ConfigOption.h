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
#include "Common/Compiler.h"

class ConfigOptionUI;

class ConfigOption{
public:
    virtual ~ConfigOption() = default;

    virtual void load_json(const QJsonValue& json) = 0;
    virtual QJsonValue to_json() const = 0;

    //  Returns error message if invalid. Otherwise returns empty string.
    virtual QString check_validity() const{ return QString(); };

    virtual void restore_defaults(){};

    virtual ConfigOptionUI* make_ui(QWidget& parent) = 0;
};


class ConfigOptionUI{
public:
    virtual ~ConfigOptionUI() = default;
    virtual QWidget* widget() = 0;
    virtual void restore_defaults() = 0;
};



#endif
