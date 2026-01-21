/*  Locations
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Locations_H
#define PokemonAutomation_PokemonLZA_Locations_H

#include <vector>
#include <map>
#include "CommonFramework/Language.h"
#include "Common/Cpp/EnumStringMap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

enum class Location{
    CENTRICO_PLAZA,
    GARE_DE_LUMIOSE,
    POKEMON_RESEARCH_LAB,
    HOTEL_Z,
    VERT_POKEMON_CENTER,
    RACINE_CONSTRUCTION,
    RESTAURANT_LE_NAH,
    CAFE_CYCLONE,
    CAFE_CLASSE,
    CAFE_INTROVERSION,
    NOUVEAU_CAFE,
    RUST_SYNDICATE_OFFICE,
    LUMIOSE_SEWERS_1,
    BLEU_POKEMON_CENTER,
    VERNAL_POKEMON_CENTER,
    CAFE_WOOF,
    CAFE_SOLEIL,
    SHUTTERBUG_CAFE,
    NOUVEAU_CAFE_TRUCK_NO_2,
    QUASARTICO_INC,
    LYSANDRE_CAFE,
    LUMIOSE_SEWERS_2,
    MAGENTA_POKEMON_CENTER,
    MAGENTA_PLAZA_POKEMON_CENTER,
    CAFE_POKEMON_AMIE,
    CAFE_ROULEAU,
    CAFE_GALLANT,
    CAFE_TRISTE,
    NOUVEAU_CAFE_TRUCK_NO_3,
    HOTEL_RICHISSIME,
    LOOKER_BUREAU,
    LUMIOSE_MUSEUM,
    ROUGE_POKEMON_CENTER,
    CENTRICO_POKEMON_CENTER,
    RESTAURANT_LE_YEAH,
    SUSHI_HIGH_ROLLER,
    RESTAURANT_LE_WOW,
    JUSTICE_DOJO,
    JAUNE_POKEMON_CENTER,
    HIBERNAL_POKEMON_CENTER,
    CAFE_ULTIMO,
    CAFE_ACTION,
    CAFE_KIZUNA,
    CAFE_BATAILLE,
    WILD_ZONE_1,
    WILD_ZONE_2,
    WILD_ZONE_3,
    WILD_ZONE_4,
    WILD_ZONE_5,
    WILD_ZONE_6,
    WILD_ZONE_7,
    WILD_ZONE_8,
    WILD_ZONE_9,
    WILD_ZONE_10,
    WILD_ZONE_11,
    WILD_ZONE_12,
    WILD_ZONE_13,
    WILD_ZONE_14,
    WILD_ZONE_15,
    WILD_ZONE_16,
    WILD_ZONE_17,
    WILD_ZONE_18,
    WILD_ZONE_19,
    WILD_ZONE_20_NO_DISTORTION,
    WILD_ZONE_20_WITH_DISTORTION,
    HYPERSPACE_ENTRY_POINT
};

enum class FAST_TRAVEL_FILTER{
    ALL_TRAVEL_SPOTS,
    FACILITIES,
    POKEMON_CENTERS,
    CAFES,
    ZONES
};

struct LocationItem{
    Location location;
    std::string slug;
    uint8_t index;
};

class Locations{
public:
    const std::string& display_name() const { return m_display_name; }
private:
    friend struct LocationNameDatabase;

    // The english in-game name of a location
    std::string m_display_name;
    // A map between a language and the location in-game name in that language
    std::map<Language, std::string> m_display_names;
};

// Search the LocationNameDatabase for a Locations object using its slug
const Locations& get_location_name(const std::string& slug);

// Search for the slug of a location given its English display name
const std::string& parse_location_name(const std::string& display_name);

// A list of all location slugs
const std::vector<std::string>& LOCATION_SLUGS();

// Map between Location enums and the slug for ease-of-use elsewhere
const std::vector<LocationItem>& LOCATION_ENUM_MAPPINGS();
const LocationItem& get_location_item_from_enum(const Location location);
const LocationItem& get_location_item_from_slug(const std::string& slug);

}
}
}
#endif