/*  TeraTypeDetector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraTypeDetector_H
#define PokemonAutomation_PokemonSV_TeraTypeDetector_H

#include <array>

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

enum class TeraType {
	Bug,
	Dark,
	Dragon,
	Electric,
	Fairy,
	Fighting,
	Fire,
	Flying,
	Ghost,
	Grass,
	Ground,
	Ice,
	Normal,
	Poison,
	Psychic,
	Rock,
	Steel,
	Water,
};

constexpr size_t NUM_TERA_TYPE = 18;

extern const std::array<std::string, NUM_TERA_TYPE> TERA_TYPE_NAMES;

TeraType detect_tera_type(Logger& logger, const ImageViewRGB32& screen);

void CREATE_ICONS_FROM_SCREEN();

}
}
}
#endif
