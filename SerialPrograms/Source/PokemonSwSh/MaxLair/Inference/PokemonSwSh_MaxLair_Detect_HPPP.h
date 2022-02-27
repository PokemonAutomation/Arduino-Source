/*  Max Lair Detect HP and PP
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_HPPP_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_HPPP_H

#include <stdint.h>
#include <QImage>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//double read_hp_bar(const QImage& image);

double read_hp_bar(LoggerQt& logger, const QImage& image);
Health read_in_battle_hp_box(LoggerQt& logger, const QImage& sprite, const QImage& hp_bar);
int8_t read_pp_text(LoggerQt& logger, QImage image);


}
}
}
}
#endif
