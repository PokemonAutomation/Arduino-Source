/*  Resolution Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Options_ResolutionOption_H
#define PokemonAutomation_Options_ResolutionOption_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/GroupOption.h"

namespace PokemonAutomation{


class ResolutionOption : public GroupOption{
public:
    ResolutionOption(
        std::string label, std::string description,
        int default_width, int default_height,
        int initial_x_pos, int initial_y_pos
    );

    StaticTextOption DESCRIPTION;
    SimpleIntegerOption<uint32_t> WIDTH;
    SimpleIntegerOption<uint32_t> HEIGHT;
    SimpleIntegerOption<int32_t> X_POS;
    SimpleIntegerOption<int32_t> Y_POS;
};


}
#endif
