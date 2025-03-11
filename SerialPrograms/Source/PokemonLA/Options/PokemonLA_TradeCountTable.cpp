/*  Trade Count Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonSprites.h"
#include "PokemonLA_TradeCountTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



std::map<std::string, uint8_t> make_research_catch_count_map(){
    std::string path = RESOURCE_PATH() + "PokemonLA/ResearchMaxCatches.json";
    JsonValue json = load_json_file(path);
    JsonObject& root = json.to_object_throw(path);

    std::map<std::string, uint8_t> map;
    for (auto& item : root){
        map.emplace(item.first, (uint8_t)item.second.to_integer_throw(path));
    }

    return map;
}
uint8_t research_catch_count(const std::string& slug){
    static const std::map<std::string, uint8_t> database = make_research_catch_count_map();
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No research count for: " + slug);
    }
    return iter->second;
}




TradeCountTableRow::TradeCountTableRow(const std::string& slug, const ImageViewRGB32& icon)
    : StaticTableRow(slug)
    , default_value(research_catch_count(slug))
    , pokemon(LockMode::LOCK_WHILE_RUNNING, Pokemon::get_pokemon_name(slug).display_name(), icon, 40)
    , count(LockMode::LOCK_WHILE_RUNNING, default_value, 0, default_value)
    , default_label(LockMode::LOCK_WHILE_RUNNING, std::to_string(default_value), ImageViewRGB32())
{
    PA_ADD_STATIC(pokemon);
    add_option(count, "Count");
    PA_ADD_STATIC(default_label);
}

TradeCountTable::TradeCountTable()
    : StaticTableOption(
        "<b>Trade Counts:</b>"
        "<br>Trade each " + STRING_POKEMON + " of this species this many times.<br>"
        "The defaults here are the # of catches needed to max out research. "
        "Maxing out catches is sufficient to reach level 10 for everything except Unown, Spritomb, and legendaries. "
        "Note that gen1 trade evolutions cannot be touch traded. The program will skip them. This applies to Kadabra, Haunter, Graveler, and Machoke.",
        LockMode::LOCK_WHILE_RUNNING
    )
{
    for (const std::string& slug : HISUI_DEX_SLUGS()){
        const SpriteDatabase::Sprite* sprite = ALL_POKEMON_SPRITES().get_nothrow(slug);
        ImageViewRGB32 icon;
        if (sprite != nullptr){
            icon = sprite->icon;
        }
        add_row(std::make_unique<TradeCountTableRow>(slug, icon));
    }
    finish_construction();
}
std::vector<std::string> TradeCountTable::make_header() const{
    std::vector<std::string> ret{
        STRING_POKEMON,
        "Trades",
        "Default"
    };
    return ret;
}





}
}
}
