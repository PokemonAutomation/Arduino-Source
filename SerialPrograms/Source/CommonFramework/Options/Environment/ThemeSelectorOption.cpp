/*  Theme Selector Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QTextStream>
#include <QApplication>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/EnumDropdownWidget.h"
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
    : EnumDropdownOption(
        "<b>Theme:</b>",
        {
            "Default",
            "Dark Mode",
        },
        0
    )
{}
void ThemeSelectorOption::load_json(const QJsonValue& json){
    EnumDropdownOption::load_json(json);
    set_theme(*this);
}


class ThemeSelectorWidget : public EnumDropdownWidget{
public:
    ThemeSelectorWidget(QWidget& parent, EnumDropdownOption& value)
        : EnumDropdownWidget(parent, value)
    {
        connect(
            m_box, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [&](int index){
                if (0 <= index && index <= 1){
                    set_theme(index);
                    m_value.set(index);
                }
            }
        );
    }
};




ConfigWidget* ThemeSelectorOption::make_ui(QWidget& parent){
    return new ThemeSelectorWidget(parent, *this);
}


#if 1
Color theme_friendly_darkblue(){
    if (current_theme == 1){
        return Color(0xff0080ff);
    }
    return COLOR_DARK_BLUE;
}
#endif
QString html_color_text(const QString& text, Color color){
    const char HEX[] = "0123456789abcdef";
    uint32_t rgb = (uint32_t)color;
    QString str;
    str += HEX[(rgb >> 20) & 15];
    str += HEX[(rgb >> 16) & 15];
    str += HEX[(rgb >> 12) & 15];
    str += HEX[(rgb >>  8) & 15];
    str += HEX[(rgb >>  4) & 15];
    str += HEX[(rgb >>  0) & 15];
    return "<font color=#" + str + ">" + text + "</font>";
}
QString make_text_url(const QString& url, const QString& text){
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
