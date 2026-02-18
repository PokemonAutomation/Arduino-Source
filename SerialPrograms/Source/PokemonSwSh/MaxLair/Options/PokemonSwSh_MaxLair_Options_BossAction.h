/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Options_BossAction_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StaticTableOption.h"
#include "CommonFramework/Options/LabelCellOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


enum class BossAction{
    CATCH_AND_STOP_PROGRAM,
    CATCH_AND_STOP_IF_SHINY,
};


class BossActionRow : public StaticTableRow,
                        public OptionListener<EnumDropdownOption<BossAction>>{
public:
    BossActionRow(std::string slug, const std::string& name_slug, const std::string& sprite_slug);
                            
    virtual void value_changed(void* object, const EnumDropdownOption<BossAction>& option, BossAction value) override;

    LabelCellOption pokemon;
    EnumDropdownOption<BossAction> action;
    PokemonBallSelectCell ball;
    BooleanCheckBoxOption save_on_the_go;
};

class BossActionTable : public StaticTableOption{
public:
    BossActionTable();
    virtual std::vector<std::string> make_header() const override;
    
private:
    std::vector<BossActionRow*> m_rows;
};





}
}
}
}
#endif
