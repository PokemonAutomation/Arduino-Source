/*  DPI Scaler
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DpiScaler_H
#define PokemonAutomation_DpiScaler_H

#include <QApplication>
#include <QScreen>

namespace PokemonAutomation{


int scale_dpi_width(int width);
int scale_dpi_height(int height);


QString scale_dpi_stylesheet(const QString& style_str);



}
#endif
