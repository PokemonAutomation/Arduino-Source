/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_NonBossAction_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_NonBossAction_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "CommonFramework/Options/LabelCellOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


enum class NonBossAction{
    IGNORE,
    STOP_PROGRAM,
};

class NonBossActionRow : public StaticTableRow{
public:
    NonBossActionRow(std::string slug, const std::string& name_slug, const std::string& sprite_slug);

    LabelCellOption pokemon;
    EnumDropdownCell<NonBossAction> action;
};

class NonBossActionTable : public StaticTableOption{
public:
    NonBossActionTable();
    virtual std::vector<std::string> make_header() const;
};





}
}
}
}
#endif
