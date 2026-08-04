/*  Colored Text
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_ColoredText_H
#define PokemonAutomation_ColoredText_H

#include "Common/Cpp/Color.h"
#include <string>

namespace PokemonAutomation{

enum class UiThemeMode{
    DEFAULT_MODE,
    DARK_MODE,
};

inline UiThemeMode CURRENT_THEME = UiThemeMode::DEFAULT_MODE;

Color theme_friendly_darkblue();
std::string html_color_text(const std::string& text, Color color);
std::string make_text_url(const std::string& url, const std::string& text);

}
#endif

