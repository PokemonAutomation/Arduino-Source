/*  DPI Scaler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QApplication>
#include <QScreen>
#include "DpiScaler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


int scale_dpi_width(int width){
    int dpi = 96;
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen != nullptr){
        dpi = primaryScreen->logicalDotsPerInchX();
    }
    return width * dpi / 96;
}
int scale_dpi_height(int height){
    int dpi = 96;
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen != nullptr){
        dpi = primaryScreen->logicalDotsPerInchY();
    }
    return height * dpi / 96;
}


void scale_dpi_token(QString& str, int dpi){
    size_t length = str.length();
    if (length < 3){
        return;
    }
    if (str[length - 2] != 'p' || str[length - 1] != 'x'){
        return;
    }

    //  Parse the integer in front.
    int64_t value = 0;
    for (size_t c = 0; c < length - 2; c++){
        QChar ch = str[c];
        if (!('0' <= ch && ch <= '9')){
            return;
        }
        value *= 10;
        value += ch.unicode() - '0';
    }
    value *= dpi;
    value /= 96;
    str = QString::number(value) + "px";
}


QString scale_dpi_stylesheet(const QString& style_str){
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen == nullptr){
        return style_str;
    }
    int dpi_x = primaryScreen->logicalDotsPerInchX();
    int dpi_y = primaryScreen->logicalDotsPerInchY();
    int dpi = std::max(dpi_x, dpi_y);

    QString out;

//    cout << "style_str.length() = " <<style_str.length() << endl;

    QString current;
    for (QChar ch : style_str){
        if (ch.isLetterOrNumber()){
            current += ch;
            continue;
        }

        //  End of token

        scale_dpi_token(current, dpi);

        current += ch;
        out += current;
        current.clear();
    }
    out += current;
//    cout << "out.length() = " << out.length() << endl;
    return out;
}


}
