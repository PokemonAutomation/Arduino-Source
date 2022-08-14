/*  Static Text Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StaticTextWidget_H
#define PokemonAutomation_Options_StaticTextWidget_H

#include <QWidget>
#include "Common/Cpp/Options/StaticTextOption.h"
#include "ConfigWidget.h"

class QLabel;

namespace PokemonAutomation{



class StaticTextWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~StaticTextWidget();
    StaticTextWidget(QWidget& parent, StaticTextOption& value);
    virtual void update_ui() override;
    virtual void restore_defaults() override;
    virtual void value_changed() override;

private:
    StaticTextOption& m_value;
    QLabel* m_text;
};


class SectionDividerWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~SectionDividerWidget();
    SectionDividerWidget(QWidget& parent, SectionDividerOption& value);
    virtual void update_ui() override;
    virtual void restore_defaults() override;
    virtual void value_changed() override;

private:
    SectionDividerOption& m_value;
    QLabel* m_text;
};




}
#endif
