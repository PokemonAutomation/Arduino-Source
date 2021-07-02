/*  Various Utilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Tools_H
#define PokemonAutomation_Tools_H

#include <stdint.h>
#include <memory>
#include <string>
#include <map>
#include <QApplication>
#include <QDate>
#include <QJsonObject>
#include <QJsonDocument>

namespace PokemonAutomation{


extern const QString STRING_POKEMON;



bool valid_switch_date(const QDate& date);



//  Build the .hex
int build_hexfile(
    const std::string& board,
    const QString& category,
    const QString& program_name,
    const QString& hex_file,
    const QString& log_file
);



extern std::unique_ptr<QApplication> application;

template <typename Lambda>
void run_on_main_thread(Lambda&& lambda){
    QMetaObject::invokeMethod(application.get(), std::move(lambda));
}


}
#endif
