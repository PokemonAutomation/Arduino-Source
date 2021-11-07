/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StaticText_H
#define PokemonAutomation_StaticText_H

#include <QJsonValue>
#include <QCheckBox>
#include "ConfigOption.h"

namespace PokemonAutomation{



class StaticTextOption : public ConfigOption{
public:
    StaticTextOption(QString label);
    virtual void load_json(const QJsonValue& json) override{}
    virtual QJsonValue to_json() const override{ return QJsonValue(); }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class StaticTextOptionUI;
    QString m_label;
};
class StaticTextOptionUI : public QWidget, public ConfigOptionUI{
public:
    StaticTextOptionUI(QWidget& parent, StaticTextOption& value);
    virtual void restore_defaults() override{}
};



class SectionDividerOption : public ConfigOption{
public:
    SectionDividerOption(QString label);
    virtual void load_json(const QJsonValue& json) override{}
    virtual QJsonValue to_json() const override{ return QJsonValue(); }

    virtual ConfigOptionUI* make_ui(QWidget& parent) override;

private:
    friend class SectionDividerOptionUI;
    QString m_label;
};
class SectionDividerOptionUI : public QWidget, public ConfigOptionUI{
public:
    SectionDividerOptionUI(QWidget& parent, SectionDividerOption& value);
    virtual void restore_defaults() override{}
};





}
#endif


