/*  Theme Selector Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QTextStream>
#include <QApplication>
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
        stylesheet = ":qdarkstyle/dark/style.qss";
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






}
