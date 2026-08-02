/*  Theme Selector Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFile>
#include <QTextStream>
#include <QApplication>
#include "Common/Cpp/ColoredText.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "ThemeSelectorOption.h"

namespace PokemonAutomation{



void set_theme(size_t index){
    if (index == CURRENT_THEME){
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
        if (!f.open(QFile::ReadOnly | QFile::Text)){
            return;
        }
        QTextStream ts(&f);
        stylesheet = scale_dpi_stylesheet(ts.readAll());
    }

    QApplication* app = static_cast<QApplication*>(QApplication::instance());
    app->setStyleSheet(stylesheet);

    CURRENT_THEME = index;
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






}
