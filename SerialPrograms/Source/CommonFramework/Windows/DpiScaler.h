/*  DPI Scaler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DpiScaler_H
#define PokemonAutomation_DpiScaler_H

#include <QApplication>
#include <QDesktopWidget>

namespace PokemonAutomation{


inline int scale_dpi_width(int width){
    return width * QApplication::desktop()->logicalDpiX() / 96;
}
inline int scale_dpi_height(int height){
    return height * QApplication::desktop()->logicalDpiY() / 96;
}


}
#endif
