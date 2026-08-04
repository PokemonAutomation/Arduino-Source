/*  Colored Text
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include "ColoredText.h"

namespace PokemonAutomation{


#if 1
Color theme_friendly_darkblue(){
    if (CURRENT_THEME == UiThemeMode::DARK_MODE){
        return Color(0xff0080ff);
    }
    return COLOR_DARK_BLUE;
}
#endif
std::string html_color_text(const std::string& text, Color color){
    if (color == Color()){
        return text;
    }
    const char HEX[] = "0123456789abcdef";
    uint32_t rgb = (uint32_t)color;
    std::string str;
    str += HEX[(rgb >> 20) & 15];
    str += HEX[(rgb >> 16) & 15];
    str += HEX[(rgb >> 12) & 15];
    str += HEX[(rgb >>  8) & 15];
    str += HEX[(rgb >>  4) & 15];
    str += HEX[(rgb >>  0) & 15];
    return "<font color=#" + str + ">" + text + "</font>";
}
std::string make_text_url(const std::string& url, const std::string& text){
#if 0
    switch (CURRENT_THEME){
    case 0:
        return "<a href=\"" + url + "\">" + text + "</a>";
    case 1:
        return "<a href=\"" + url + "\" style=\"color: #0080ff\">" + text + "</a>";
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid theme #.");
#endif
    return "<a href=\"" + url + "\" style=\"color: #0080ff\">" + text + "</a>";
}






}
