/*  Floating-Point Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include "Common/Qt/Options/FloatingPoint/FloatingPointBaseWidget.h"
#include "FloatingPointOption.h"

namespace PokemonAutomation{



FloatingPointOption::FloatingPointOption(
    QString label,
    double default_value,
    double min_value,
    double max_value
)
    : FloatingPointBaseOption(std::move(label), min_value, max_value, default_value)
{}

void FloatingPointOption::load_json(const QJsonValue& json){
    return this->load_current(json);
}
QJsonValue FloatingPointOption::to_json() const{
    return this->write_current();
}

QString FloatingPointOption::check_validity() const{
    return FloatingPointBaseOption::check_validity();
}
void FloatingPointOption::restore_defaults(){
    FloatingPointBaseOption::restore_defaults();
}




class FloatingPointOptionUI : private FloatingPointBaseWidget, public ConfigWidget{
public:
    FloatingPointOptionUI(QWidget& parent, FloatingPointOption& value)
        : FloatingPointBaseWidget(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        FloatingPointBaseWidget::restore_defaults();
    }
};


ConfigWidget* FloatingPointOption::make_ui(QWidget& parent){
    return new FloatingPointOptionUI(parent, *this);
}



}
