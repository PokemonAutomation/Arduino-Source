/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_Options_StringSelectWidget_H
#define PokemonAutomation_CommonTools_Options_StringSelectWidget_H

#include <QWidget>
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{


class StringSelectCellWidget : public NoWheelComboBox, public ConfigWidget{
public:
    ~StringSelectCellWidget();
    StringSelectCellWidget(QWidget& parent, StringSelectCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

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

private:
    StringSelectCellWidget* m_cell;
};



}
#endif
