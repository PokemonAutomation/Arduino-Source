/*  DPI Scaler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DpiScaler_H
#define PokemonAutomation_DpiScaler_H

#include <QApplication>
#include <QScreen>

namespace PokemonAutomation{


inline int scale_dpi_width(int width){
    int dpi = 96;
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen != nullptr){
        dpi = primaryScreen->logicalDotsPerInchX();
    }
    return width * dpi / 96;
}
inline int scale_dpi_height(int height){
    int dpi = 96;
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen != nullptr){
        dpi = primaryScreen->logicalDotsPerInchY();
    }
    return height * dpi / 96;
}


}
#endif
