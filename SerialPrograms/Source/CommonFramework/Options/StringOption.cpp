/*  String Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/String/StringBaseWidget.h"
#include "StringOption.h"

namespace PokemonAutomation{



StringOption::StringOption(
    bool is_password,
    std::string label,
    std::string default_value,
    std::string placeholder_text
)
    : StringBaseOption(
        is_password,
        std::move(label),
        default_value,
        std::move(placeholder_text)
    )
{}


void StringOption::load_json(const JsonValue& json){
    load_current(json);
}
JsonValue StringOption::to_json() const{
    return write_current();
}

void StringOption::restore_defaults(){
    StringBaseOption::restore_defaults();
}




class StringWidget : private StringBaseWidget, public ConfigWidget{
public:
    StringWidget(QWidget& parent, StringOption& value)
        : StringBaseWidget(parent, value)
        , ConfigWidget(value, *this)
    {}
    virtual void restore_defaults() override{
        StringBaseWidget::restore_defaults();
    }
    virtual void update_ui() override{
        StringBaseWidget::update_ui();
    }
};


ConfigWidget* StringOption::make_ui(QWidget& parent){
    return new StringWidget(parent, *this);
}




}
