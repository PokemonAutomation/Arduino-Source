/*  Divider
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 *      This isn't a real option. It's just a line divider between groups of options.
 */

#ifndef PokemonAutomation_Divider_H
#define PokemonAutomation_Divider_H

#include "ConfigItem.h"

namespace PokemonAutomation{


class Divider : public ConfigItem{
public:
    static const QString OPTION_TYPE;
    static const QString JSON_TEXT;

public:
    Divider(const QJsonObject& obj);

    virtual const QString& type() const override{ return OPTION_TYPE; }
    virtual QWidget* make_ui(QWidget& parent) override;

private:
    friend class BooleanCheckBoxUI;
};



class DividerUI : public QWidget{
public:
    DividerUI(QWidget& parent, const QString& label);
    ~DividerUI();
};


}
#endif
