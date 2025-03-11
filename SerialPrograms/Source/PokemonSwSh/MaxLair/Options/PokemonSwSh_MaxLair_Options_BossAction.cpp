/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Options_BossAction.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{




const EnumDropdownDatabase<BossAction>& BossAction_Database(){
    static const EnumDropdownDatabase<BossAction> database({
        {BossAction::CATCH_AND_STOP_PROGRAM, "always-stop", "Always stop program."},
        {BossAction::CATCH_AND_STOP_IF_SHINY, "stop-if-shiny", "Stop if shiny."},
    });
    return database;
}

BossActionRow::BossActionRow(std::string slug, const std::string& name_slug, const std::string& sprite_slug)
    : StaticTableRow(std::move(slug))
    , pokemon(LockMode::LOCK_WHILE_RUNNING, get_pokemon_name(name_slug).display_name(), ALL_POKEMON_SPRITES().get_throw(sprite_slug).icon)
    , action(BossAction_Database(), LockMode::LOCK_WHILE_RUNNING, BossAction::CATCH_AND_STOP_IF_SHINY)
    , ball("poke-ball")
{
    PA_ADD_STATIC(pokemon);
    add_option(action, "Action");
    add_option(ball, "Ball");
}


BossActionTable::BossActionTable()
    : StaticTableOption("<b>Boss Actions:</b>", LockMode::LOCK_WHILE_RUNNING)
{
    for (const auto& item : all_bosses_by_dex()){
//        cout << item.second << endl;
        const MaxLairSlugs& slugs = get_maxlair_slugs(item.second);
        const std::string& sprite_slug = *slugs.sprite_slugs.begin();
        const std::string& name_slug = slugs.name_slug;
        add_row(std::make_unique<BossActionRow>(item.second, name_slug, sprite_slug));
    }
    finish_construction();
}
std::vector<std::string> BossActionTable::make_header() const{
    std::vector<std::string> ret{
        STRING_POKEMON,
        "Action",
        STRING_POKEBALL
    };
    return ret;
}






}
}
}
}
