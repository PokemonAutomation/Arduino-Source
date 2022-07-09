/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StaticTextOption_H
#define PokemonAutomation_StaticTextOption_H

#include <QString>
#include "ConfigOption.h"

namespace PokemonAutomation{



class StaticTextOption : public ConfigOption{
public:
    StaticTextOption(QString label);

    const QString& label() const{ return m_label; }

    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class StaticTextWidget;
    QString m_label;
};



class SectionDividerOption : public ConfigOption{
public:
    SectionDividerOption(QString label);
    virtual void load_json(const JsonValue2& json) override;
    virtual JsonValue2 to_json() const override;

    virtual ConfigWidget* make_ui(QWidget& parent) override;

private:
    friend class SectionDividerWidget;
    QString m_label;
};





}
#endif


