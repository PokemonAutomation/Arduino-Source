/*  Pokemon Ball Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonBallSelect_H
#define PokemonAutomation_Pokemon_PokemonBallSelect_H

#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokemonBallSelectData{
    PokemonBallSelectData(const std::vector<std::string>& slugs);
    StringSelectDatabase m_database;
};


class PokemonBallSelect : private PokemonBallSelectData, public StringSelectOption{
public:
    PokemonBallSelect(std::string label, const std::string& default_slug = "");
};


}
}
#endif
