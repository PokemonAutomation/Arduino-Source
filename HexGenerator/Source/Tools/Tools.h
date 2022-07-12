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

namespace PokemonAutomation{


extern const std::string STRING_POKEMON;



bool valid_switch_date(const QDate& date);



//  Build the .hex
int build_hexfile(
    const std::string& board,
    const std::string& category,
    const std::string& program_name,
    const std::string& hex_file,
    const std::string& log_file
);



extern std::unique_ptr<QApplication> application;

template <typename Lambda>
void run_on_main_thread(Lambda&& lambda){
    QMetaObject::invokeMethod(application.get(), std::move(lambda));
}


}
#endif
