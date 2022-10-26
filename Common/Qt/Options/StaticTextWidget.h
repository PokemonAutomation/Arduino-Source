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



class StaticTextWidget : public QWidget, public ConfigWidget{
public:
    ~StaticTextWidget();
    StaticTextWidget(QWidget& parent, StaticTextOption& value);

    virtual void update_visibility(bool program_is_running) override;
    virtual void value_changed() override;

private:
    StaticTextOption& m_value;
    QLabel* m_text;
};


class SectionDividerWidget : public QWidget, public ConfigWidget{
public:
    ~SectionDividerWidget();
    SectionDividerWidget(QWidget& parent, SectionDividerOption& value);

    virtual void update_visibility(bool program_is_running) override;
    virtual void value_changed() override;

private:
    SectionDividerOption& m_value;
    QLabel* m_text;
};




}
#endif
