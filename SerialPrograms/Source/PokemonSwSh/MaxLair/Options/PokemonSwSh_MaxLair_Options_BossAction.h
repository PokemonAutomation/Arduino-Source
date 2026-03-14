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
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include <vector>
#include <memory>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


enum class BossAction{
    CATCH_AND_STOP_PROGRAM,
    CATCH_AND_STOP_IF_SHINY,
};

const EnumDropdownDatabase<BossAction>& BossAction_Database();

class BossActionRow : public StaticTableRow,
private ConfigOption::Listener
{
public:
    BossActionRow(std::string slug, const std::string& name_slug, const std::string& sprite_slug);
    virtual void on_config_value_changed(void* object) override;

    
    LabelCellOption pokemon;
    EnumDropdownCell<BossAction> action;
    PokemonBallSelectCell ball;
    BooleanCheckBoxCell save_on_the_go;
};

class BossActionTable : public StaticTableOption,
private ConfigOption::Listener
{
public:
    BossActionTable();
    ~BossActionTable();
    
    virtual void on_config_value_changed(void* object) override;
    virtual std::vector<std::string> make_header() const override;
    
private:
    std::vector<BossActionRow*> m_rows;
    bool m_reverting;
    void update_checkbox_states();
};





}
}
}
}
#endif
