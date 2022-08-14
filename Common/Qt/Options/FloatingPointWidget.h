/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FloatingPointWidget_H
#define PokemonAutomation_FloatingPointWidget_H

#include <QWidget>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "ConfigWidget.h"

class QLineEdit;

namespace PokemonAutomation{



class FloatingPointWidget : public QWidget, public ConfigWidget, private ConfigOption::Listener{
public:
    ~FloatingPointWidget();
    FloatingPointWidget(QWidget& parent, FloatingPointOption& value);
    virtual void restore_defaults() override;
    virtual void update_ui() override;
    virtual void value_changed() override;

private:
    FloatingPointOption& m_value;
    QLineEdit* m_box;
};



}
#endif
