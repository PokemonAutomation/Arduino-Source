/*  Theme Selector Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ThemeSelectorOption_H
#define PokemonAutomation_ThemeSelectorOption_H

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{


class ThemeSelectorOption : public EnumDropdownOption{
public:
    ThemeSelectorOption();

    virtual void set(size_t index) override;
    virtual void load_json(const JsonValue& json) override;
};


Color theme_friendly_darkblue();
std::string html_color_text(const std::string& text, Color color);
std::string make_text_url(const std::string& url, const std::string& text);



}
#endif
