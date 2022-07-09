/*  Boolean Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/BooleanCheckBox/BooleanCheckBoxBaseWidget.h"
#include "BooleanCheckBoxOption.h"

namespace PokemonAutomation{



class BooleanCheckBoxWidget : private BooleanCheckBoxBaseWidget, public ConfigWidget{
public:
    BooleanCheckBoxWidget(QWidget& parent, BooleanCheckBoxOption& value)
        : BooleanCheckBoxBaseWidget(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        BooleanCheckBoxBaseWidget::restore_defaults();
    }
    virtual void update_ui() override{
        BooleanCheckBoxBaseWidget::update_ui();
    }
};




void BooleanCheckBoxOption::load_json(const JsonValue2& json){
    load_current(json);
}
JsonValue2 BooleanCheckBoxOption::to_json() const{
    return write_current();
}

void BooleanCheckBoxOption::restore_defaults(){
    BooleanCheckBoxBaseOption::restore_defaults();
}

ConfigWidget* BooleanCheckBoxOption::make_ui(QWidget& parent){
    return new BooleanCheckBoxWidget(parent, *this);
}


}
