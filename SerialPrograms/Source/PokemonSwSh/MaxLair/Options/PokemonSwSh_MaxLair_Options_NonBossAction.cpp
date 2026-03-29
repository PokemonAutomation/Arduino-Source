/*  Max Lair Non-Boss Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh_MaxLair_Options_NonBossAction.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{




const EnumDropdownDatabase<NonBossAction>& NonBossAction_Database(){
    static const EnumDropdownDatabase<NonBossAction> database({
        {NonBossAction::IGNORE, "ignore", "Ignore"},
        {NonBossAction::STOP_PROGRAM, "stop", "Stop program"},
    });
    return database;
}

static std::string format_display_name(const std::string& slug) {
    static const std::vector<std::pair<std::string, std::string>> region_map = {
        
        {"-alola", "Alolan "},
        {"-galar", "Galarian "}
    };
    
    std::string base_slug = slug;
    std::string prefix;
    
    for (const auto& pair : region_map) {
        size_t pos = slug.find(pair.first);
        if (pos != std::string::npos) {
            base_slug = slug.substr(0, pos);
            
            prefix = pair.second;
            break;
        }
    }
    
    // Get base Pokemon name
    std::string base_name = get_pokemon_name(base_slug).display_name();
    return prefix + base_name;
}

NonBossActionRow::NonBossActionRow(std::string slug, const std::string& name_slug, const std::string& sprite_slug)
    : StaticTableRow(slug)
    , pokemon(
        LockMode::UNLOCK_WHILE_RUNNING,
        format_display_name(slug),
        ALL_POKEMON_SPRITES().get_throw(sprite_slug).icon
    )
    , action(
        NonBossAction_Database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        NonBossAction::IGNORE
    )
{
    PA_ADD_STATIC(pokemon);
    add_option(action, "Action");
}


NonBossActionTable::NonBossActionTable()
    : StaticTableOption("<b>Other Pokemon Actions:</b>", LockMode::UNLOCK_WHILE_RUNNING)
{
    
    // Limited mode: only display Alolan-Raichu and Alolan-Marowak by default. If the developer wants, it can also add the full list
    const bool LIMITED_MODE = true;
    
    if (LIMITED_MODE) {
        std::vector<std::string> slugs = {"raichu-alola", "marowak-alola"};
        for (const std::string& slug : slugs) {
            const MaxLairSlugs& slugs_info = get_maxlair_slugs(slug);
            const std::string& sprite_slug = *slugs_info.sprite_slugs.begin();
            const std::string& name_slug = slugs_info.name_slug;
            add_row(std::make_unique<NonBossActionRow>(slug, name_slug, sprite_slug));
        }
    } else {
        std::set<std::string> added_grouped_base; // Track base slugs for grouped forms
        
        for (const auto& item : all_rentals_by_dex()){
            const std::string& full_slug = item.second;
            const MaxLairSlugs& slugs = get_maxlair_slugs(full_slug);
            const std::string& sprite_slug = *slugs.sprite_slugs.begin();
            const std::string& name_slug = slugs.name_slug;
            
            if (full_slug == name_slug) {
                add_row(std::make_unique<NonBossActionRow>(full_slug, name_slug, sprite_slug));
                continue;
            }
            
            // Check Alola and Galar variants
            if (full_slug.size() > 6 && full_slug.substr(full_slug.size() - 6) == "-alola") {
                add_row(std::make_unique<NonBossActionRow>(full_slug, name_slug, sprite_slug));
                continue;
            }
            if (full_slug.size() > 6 && full_slug.substr(full_slug.size() - 6) == "-galar") {
                add_row(std::make_unique<NonBossActionRow>(full_slug, name_slug, sprite_slug));
                continue;
            }
            
            // All other pokemon with suffixes (e.g. Basculin) are grouped under one name
            
            if (added_grouped_base.find(name_slug) == added_grouped_base.end()) {
                add_row(std::make_unique<NonBossActionRow>(name_slug, name_slug, sprite_slug));
                added_grouped_base.insert(name_slug);
            }
        }
    }
    finish_construction();
}

std::vector<std::string> NonBossActionTable::make_header() const{
    std::vector<std::string> ret{
        STRING_POKEMON,
        "Action",
    };
    return ret;
}






}
}
}
}
