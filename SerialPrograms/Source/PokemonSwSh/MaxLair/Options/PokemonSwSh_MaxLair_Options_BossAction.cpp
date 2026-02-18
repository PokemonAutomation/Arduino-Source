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
    , save_on_the_go(LockMode::UNLOCK_WHILE_RUNNING, "Save when seen?", false)
{
    PA_ADD_STATIC(pokemon);
    add_option(action, "Action");
    add_option(ball, "Ball");
    add_option(save_on_the_go, "Save when seen?");
    
    save_on_the_go.set_visibility(
        action.enum_value() == BossAction::CATCH_AND_STOP_IF_SHINY ? ConfigOptionState::ENABLED : ConfigOptionState::DISABLED
    );
    
    action.add_listener(*this);
}

void BossActionRow::value_changed(void* object, const EnumDropdownOption<BossAction>& option, BossAction value) {
    if (&option == &action) {
        save_on_the_go.set_visibility(
            value == BossAction::CATCH_AND_STOP_IF_SHINY ?
            ConfigOptionState::ENABLED : ConfigOptionState::DISABLED
        );
        
        if (value != BossAction::CATCH_AND_STOP_IF_SHINY) {
            save_on_the_go = false;
        }
    }
}

class BossActionTable : public StaticTableOption {
    public:
        BossActionTable();
        virtual std::vector<std::string> make_header() const override;
    private:
    std::vector<BossActionRow*> m_rows;
};


BossActionTable::BossActionTable()
    : StaticTableOption("<b>Boss Actions:</b>", LockMode::UNLOCK_WHILE_RUNNING)
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
    
    // Check function for only being able to select max 3 bosses to be saved on the go
    for (auto* row : m_rows) {
        auto& checkbox = row->save_on_the_go;
        checkbox.add_listener([this, &checkbox](const BooleanCheckBoxOption&, bool value) {
            if (value) {
                size_t count = 0;
                for (auto* r : m_rows) {
                    if (r->save_on_the_go) count++;
                }
                if (count > 3) {
                    // Make all other checkboxes equal to false
                    const_cast<BooleanCheckBoxOption&>(checkbox).set_value(false);
                }
            }
        });
    }
}
std::vector<std::string> BossActionTable::make_header() const{
    std::vector<std::string> ret{
        STRING_POKEMON,
        "Action",
        STRING_POKEBALL,
        "Save Path?"
    };
    return ret;
}






}
}
}
}
