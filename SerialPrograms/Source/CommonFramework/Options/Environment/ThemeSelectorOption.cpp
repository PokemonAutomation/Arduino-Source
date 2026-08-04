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



void set_theme(UiThemeMode theme_mode){
    if (theme_mode == CURRENT_THEME){
        return;
    }
    QString stylesheet;
    switch (theme_mode){
    case UiThemeMode::DEFAULT_MODE:
        break;
    case UiThemeMode::DARK_MODE:
        stylesheet = ":qdarkstyle/dark/darkstyle.qss";
        break;
    }

    if (!stylesheet.isEmpty()){
        QFile f(stylesheet);
        if (!f.open(QFile::ReadOnly | QFile::Text)){
            return;
        }
        QTextStream ts(&f);
        stylesheet = scale_dpi_stylesheet(ts.readAll());
    }

    QApplication* app = static_cast<QApplication*>(QApplication::instance());
    app->setStyleSheet(stylesheet);

    CURRENT_THEME = theme_mode;
}


ThemeSelectorOption::ThemeSelectorOption()
    : EnumDropdownOption<UiThemeMode>(
        "<b>Theme:</b>",
        {
            {UiThemeMode::DEFAULT_MODE, "default", "Default"},
            {UiThemeMode::DARK_MODE, "dark", "Dark Mode"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        UiThemeMode::DEFAULT_MODE
    )
{}

bool ThemeSelectorOption::set_value(size_t index){
    if (!IntegerEnumDropdownOption::set_value(index)){
        return false;
    }
    set_theme(static_cast<UiThemeMode>(index));
    return true;
}
void ThemeSelectorOption::load_json(const JsonValue& json){
    IntegerEnumDropdownOption::load_json(json);
    set_theme(static_cast<UiThemeMode>(current_value()));
}






}
