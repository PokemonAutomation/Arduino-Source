/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "Common/Qt/AutoHeightTable.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{

using namespace Pokemon;


enum class BossAction{
    CATCH_AND_STOP_PROGRAM,
    CATCH_AND_STOP_IF_SHINY,
};


class BossActionRow : public StaticTableRow{
public:
    BossActionRow(std::string slug, const std::string& name_slug, const std::string& sprite_slug);

    LabelCellOption pokemon;
    EnumDropdownCell<BossAction> action;
    PokemonBallSelectCell ball;
};

class BossActionTable : public StaticTableOption{
public:
    BossActionTable();
    virtual std::vector<std::string> make_header() const;
};





}
}
}
}
#endif
