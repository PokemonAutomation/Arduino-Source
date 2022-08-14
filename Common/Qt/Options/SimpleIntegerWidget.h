/*  Simple Integer Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SimpleIntegerWidget_H
#define PokemonAutomation_SimpleIntegerWidget_H

#include <QWidget>
#include "ConfigWidget.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

class QLineEdit;

namespace PokemonAutomation{

template <typename Type>
class SimpleIntegerWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~SimpleIntegerWidget();
    SimpleIntegerWidget(QWidget& parent, SimpleIntegerOption<Type>& value);
    virtual void restore_defaults() override;
    virtual void update_ui() override;
    virtual void value_changed() override;

private:
    SimpleIntegerOption<Type>& m_value;
    QLineEdit* m_box;
};




}
#endif
