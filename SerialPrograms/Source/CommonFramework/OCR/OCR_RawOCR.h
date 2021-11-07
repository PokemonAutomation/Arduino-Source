/*  Raw Text Recognition
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_RawOCR_H
#define PokemonAutomation_OCR_RawOCR_H

#include <QImage>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
namespace OCR{


bool language_available(Language language);

QString ocr_read(Language language, const QImage& image);



}
}
#endif
