/*  Max Lair Boss Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Compiler.h"
//#include "Common/Cpp/Json/JsonValue.h"
//#include "Common/Cpp/Json/JsonArray.h"
//#include "Common/Cpp/Json/JsonObject.h"
//#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
//#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Options_BossAction.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/ConfigOption.h"
#include <vector>
#include <memory>

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
    , pokemon(
        LockMode::UNLOCK_WHILE_RUNNING,
        get_pokemon_name(name_slug).display_name(),
        ALL_POKEMON_SPRITES().get_throw(sprite_slug).icon
    )
    , action(
        BossAction_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        BossAction::CATCH_AND_STOP_IF_SHINY
    )
    , ball(LockMode::UNLOCK_WHILE_RUNNING, "poke-ball")
    , save_on_the_go(LockMode::UNLOCK_WHILE_RUNNING, false)
{
    PA_ADD_STATIC(pokemon);
    add_option(action, "Action");
    add_option(ball, "Ball");
    add_option(save_on_the_go, "Save Path");
    
    save_on_the_go.set_visibility(
        action == BossAction::CATCH_AND_STOP_IF_SHINY ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED
                                  );
    
    action.add_listener(*this);
}

void BossActionRow::on_config_value_changed(void* object) {
    if (action != BossAction::CATCH_AND_STOP_IF_SHINY) {
        save_on_the_go = false;
    }
}


BossActionTable::BossActionTable()
    : StaticTableOption("<b>Boss Actions:</b>", LockMode::UNLOCK_WHILE_RUNNING)
    , m_reverting(false)
{
    for (const auto& item : all_bosses_by_dex()){
//        cout << item.second << endl;
        const MaxLairSlugs& slugs = get_maxlair_slugs(item.second);
        const std::string& sprite_slug = *slugs.sprite_slugs.begin();
        const std::string& name_slug = slugs.name_slug;
        
        auto row = std::make_unique<BossActionRow>(item.second, name_slug, sprite_slug);
        m_rows.push_back(row.get());
        add_row(std::move(row));
    }
    finish_construction();
    
    for (auto* row : m_rows) {
        row->save_on_the_go.add_listener(*this);
        row->action.add_listener(*this);
    }
    
    update_checkbox_states();
}

BossActionTable::~BossActionTable(){
    for (auto* row : m_rows) {
        row->save_on_the_go.remove_listener(*this);
        row->action.remove_listener(*this);
    }
}

void BossActionTable::update_checkbox_states() {
    size_t checked = 0;
    for (auto* row : m_rows) {
        if (row->save_on_the_go) checked++;
    }
    for (auto* row : m_rows) {
        bool action_ok = (row->action == BossAction::CATCH_AND_STOP_IF_SHINY);
        bool disable_by_max = (checked >= 3 && !row->save_on_the_go);
        ConfigOptionState state = (action_ok && !disable_by_max) ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED;
        row->save_on_the_go.set_visibility(state);
    }
}

void BossActionTable::on_config_value_changed(void* object) {
    if (m_reverting) return;
    
    // Counting how many checkboxes are currently checked
    size_t checked = 0;
    for (auto* row : m_rows) {
        if (row->save_on_the_go) checked++;
    }
    
    // If we exceed the 3 boxes ticked, we revert the change for the last box ticked
    if (checked > 3) {
        for (auto* row : m_rows) {
            if (object == &row->save_on_the_go && row->save_on_the_go) {
                m_reverting = true;
                row->save_on_the_go = false;
                m_reverting = false;
                
                return;
            }
        }
    }
    update_checkbox_states();
}

std::vector<std::string> BossActionTable::make_header() const{
    std::vector<std::string> ret{
        STRING_POKEMON,
        "Action",
        STRING_POKEBALL,
        "Save Path (Max 3)"
    };
    return ret;
}






}
}
}
}
