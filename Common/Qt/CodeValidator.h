/*  Raid/Trade Code Validator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CodeValidator_H
#define PokemonAutomation_CodeValidator_H

#include <QString>

namespace PokemonAutomation{

bool validate_code(size_t digits, const QString& code);
QString sanitize_code(size_t digits, const QString& code);


}
#endif
