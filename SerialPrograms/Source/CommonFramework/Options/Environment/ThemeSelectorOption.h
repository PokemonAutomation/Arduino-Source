/*  Theme Selector Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ThemeSelectorOption_H
#define PokemonAutomation_ThemeSelectorOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{


class ThemeSelectorOption : public IntegerEnumDropdownOption{
public:
    ThemeSelectorOption();

    virtual bool set_value(size_t value) override;
    virtual void load_json(const JsonValue& json) override;
};





}
#endif
