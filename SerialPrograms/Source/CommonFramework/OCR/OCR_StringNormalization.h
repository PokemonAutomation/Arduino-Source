/*  String Normalization
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_StringNormalization_H
#define PokemonAutomation_OCR_StringNormalization_H

#include <QString>

namespace PokemonAutomation{
namespace OCR{

QString remove_white_space(const QString& text);
QString remove_non_alphanumeric(const QString& text);
bool strip_leading_trailing_non_alphanumeric(QString& text);

QString run_character_reductions(const QString& text);

QString normalize(QString text);



}
}
#endif
