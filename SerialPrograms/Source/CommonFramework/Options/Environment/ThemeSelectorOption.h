/*  Theme Selector Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ThemeSelectorOption_H
#define PokemonAutomation_ThemeSelectorOption_H

#include "CommonFramework/Options/EnumDropdownOption.h"

namespace PokemonAutomation{


class ThemeSelectorOption : public EnumDropdownOption{
public:
    ThemeSelectorOption();

    virtual void load_json(const QJsonValue& json) override;

private:
    virtual ConfigWidget* make_ui(QWidget& parent) override;
};



}
#endif
