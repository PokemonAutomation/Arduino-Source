/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Options_StringSelectWidget_H
#define PokemonAutomation_Options_StringSelectWidget_H

#include <QWidget>
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{


class StringSelectCellWidget : public NoWheelComboBox, public ConfigWidget{
public:
    ~StringSelectCellWidget();
    StringSelectCellWidget(QWidget& parent, StringSelectCell& value);

    virtual void update_value() override;
    virtual void update_visibility(bool program_is_running) override;
    virtual void value_changed() override;

private:
    void load_options();
    void hide_options();

    virtual QSize sizeHint() const override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    StringSelectCell& m_value;
};



class StringSelectOptionWidget : public QWidget, public ConfigWidget{
public:
    StringSelectOptionWidget(QWidget& parent, StringSelectOption& value);

    virtual void update_visibility(bool program_is_running) override;

private:
    StringSelectCellWidget* m_cell;
};



}
#endif
