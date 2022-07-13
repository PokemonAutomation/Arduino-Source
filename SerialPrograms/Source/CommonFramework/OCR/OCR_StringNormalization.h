/*  String Normalization
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_StringNormalization_H
#define PokemonAutomation_OCR_StringNormalization_H

#include <string>
#include <QString>

namespace PokemonAutomation{
namespace OCR{

//QString remove_white_space(const QString& text);

//QString remove_non_alphanumeric(const QString& text);
std::u32string remove_non_alphanumeric(const std::u32string& text);

//bool strip_leading_trailing_non_alphanumeric(QString& text);

//QString run_character_reductions(const QString& text);
std::u32string run_character_reductions(const std::u32string& text);

//QString normalize(QString text);
std::u32string normalize_utf32(const std::string& text);



}
}
#endif
