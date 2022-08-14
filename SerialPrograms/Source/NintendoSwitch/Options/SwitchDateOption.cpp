/*  Switch Date
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "Common/Qt/Options/SwitchDate/SwitchDateBaseWidget.h"
#include "SwitchDateOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SwitchDateWidget : private SwitchDateBaseWidget, public ConfigWidget{
public:
    SwitchDateWidget(QWidget& parent, SwitchDateOption& value)
        : SwitchDateBaseWidget(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        SwitchDateBaseWidget::restore_defaults();
    }
    virtual void update_ui() override{
        SwitchDateBaseWidget::update_ui();
    }
};



SwitchDateOption::SwitchDateOption(
    std::string label,
    QDate default_value
)
    : SwitchDateBaseOption(std::move(label), default_value)
{}
void SwitchDateOption::load_json(const JsonValue& json){
    return this->load_current(json);
}
JsonValue SwitchDateOption::to_json() const{
    return this->write_current();
}

std::string SwitchDateOption::check_validity() const{
    return SwitchDateBaseOption::check_validity();
}
void SwitchDateOption::restore_defaults(){
    SwitchDateBaseOption::restore_defaults();
}


ConfigWidget* SwitchDateOption::make_ui(QWidget& parent){
    return new SwitchDateWidget(parent, *this);
}



}
}
