/*  Button Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_ButtonWidget_H
#define PokemonAutomation_Options_ButtonWidget_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "Common/Cpp/Options/ButtonOption.h"
#include "ConfigWidget.h"

namespace PokemonAutomation{



class ButtonCellWidget : public QWidget, public ConfigWidget{
public:
    ~ButtonCellWidget();
    ButtonCellWidget(QWidget& parent, ButtonCell& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    ButtonCell& m_value;
    QPushButton* m_button;
};





class ButtonOptionWidget : public QWidget, public ConfigWidget{
public:
    ~ButtonOptionWidget();
    ButtonOptionWidget(QWidget& parent, ButtonOption& value);

    virtual void update_value() override;
    virtual void on_config_value_changed(void* object) override;

private:
    ButtonOption& m_value;
    QLabel* m_label;
    QPushButton* m_button;
};




}
#endif
