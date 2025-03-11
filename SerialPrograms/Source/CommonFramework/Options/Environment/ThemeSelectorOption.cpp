/*  Theme Selector Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFile>
#include <QTextStream>
#include <QApplication>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "ThemeSelectorOption.h"

namespace PokemonAutomation{


size_t current_theme = 0;


void set_theme(size_t index){
    if (index == current_theme){
        return;
    }
    QString stylesheet;
    switch (index){
    case 0:
        break;
    case 1:
        stylesheet = ":qdarkstyle/dark/darkstyle.qss";
        break;
    }

    if (!stylesheet.isEmpty()){
        QFile f(stylesheet);
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        stylesheet = scale_dpi_stylesheet(ts.readAll());
    }

    QApplication* app = static_cast<QApplication*>(QApplication::instance());
    app->setStyleSheet(stylesheet);

    current_theme = index;
}


ThemeSelectorOption::ThemeSelectorOption()
    : IntegerEnumDropdownOption(
        "<b>Theme:</b>",
        {
            {0, "default", "Default"},
            {1, "dark", "Dark Mode"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
{}

bool ThemeSelectorOption::set_value(size_t index){
    if (!IntegerEnumDropdownOption::set_value(index)){
        return false;
    }
    set_theme(index);
    return true;
}
void ThemeSelectorOption::load_json(const JsonValue& json){
    IntegerEnumDropdownOption::load_json(json);
    set_theme(current_value());
}



#if 1
Color theme_friendly_darkblue(){
    if (current_theme == 1){
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
    switch (current_theme){
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
