/*  Section Divider
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SectionDivider_H
#define PokemonAutomation_SectionDivider_H

#include <QJsonValue>
#include <QCheckBox>
#include "ConfigOption.h"

class SectionDivider : public ConfigOption{
public:
    SectionDivider(QString label);
    virtual void load_json(const QJsonValue& json) override{}
    virtual QJsonValue to_json() const override{ return QJsonValue(); }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class SectionDividerUI;
};


class SectionDividerUI : public ConfigOptionUI, public QWidget{
public:
    SectionDividerUI(QWidget& parent, SectionDivider& value);
    virtual QWidget* widget() override{ return this; }
    virtual void restore_defaults() override{}
};


#endif


