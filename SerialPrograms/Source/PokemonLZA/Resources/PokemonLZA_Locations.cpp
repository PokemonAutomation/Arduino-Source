/*  Locations
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonLZA_Locations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

struct LocationNameDatabase{
public:
    LocationNameDatabase();

    static const LocationNameDatabase& instance(){
        static LocationNameDatabase database;
        return database;
    }

    std::map<std::string, Locations> database;
    std::map<std::string, std::string> reverse_lookup;
    std::vector<std::string> slugs;
private:
    void create_database(const std::string& path);
    void create_reverse_lookup(const std::string& path);
};

// Form the location name database with all languages, mapping slugs to display names by parsing the JSON
// The JSON resembles the following:
// "eng": {
//   "location-1-slug": [ "Location 1 Display Name" ],
//   ...
// },
// ...
LocationNameDatabase::LocationNameDatabase(){
    std::string path = RESOURCE_PATH() + "PokemonLZA/LocationName.json";
    create_database(path);
    create_reverse_lookup(path);
}

// Create the database that maps slugs to display names in various languages
void LocationNameDatabase::create_database(const std::string& path){
    // Load the JSON file containing location names in all languages from Resources area
    JsonValue json = load_json_file(path);
    JsonObject& object = json.to_object_throw(path); // Whole JSON object
    // Iterate through each language in the whole JSON
    for (const auto& language_block : object){
        Language language = language_code_to_enum(language_block.first);
        const JsonObject& language_object = language_block.second.to_object_throw(path);
        // For each language, iterate through each slug in the respective language block
        for (const auto& slug : language_object){
            // Each display name is wrapped in an array, so the display name is extracted from the array
            const JsonArray& names = slug.second.to_array_throw(path);
            if (names.empty()){
                throw JsonParseException(path, "Expected at least one name for: " + language_block.first + " : " + slug.first);
            }
            database[slug.first].m_display_names[language] = names[0].to_string_throw(path);
        }
    }
}

// Create the reverse lookup table that allows for getting slugs from display names
// Also register each slug into a vector
void LocationNameDatabase::create_reverse_lookup(const std::string& path){
    // Set the English display name as the default display name for each location
    // Build a reverse lookup table using the English display name
    for (auto& item : database){
        auto iter = item.second.m_display_names.find(Language::English);
        if (iter == item.second.m_display_names.end()){
            throw JsonParseException(path, "Expected English name for location slug: " + item.first);
        }
        item.second.m_display_name = iter->second;
        // reverse_lookup[iter->second] = item.first;
        slugs.push_back(item.first);
    }
}

// Get the Locations object given its slug using the database
const Locations& get_location_name(const std::string& slug){
    const std::map<std::string, Locations>& database = LocationNameDatabase::instance().database;
    auto iter = database.find(slug);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Location slug not found in database: " + slug
        );
    }
    return iter->second;
}

// Using the reverse lookup table, get the slug given the English display name
const std::string& parse_location_name(const std::string& display_name){
    const std::map<std::string, std::string>& database = LocationNameDatabase::instance().reverse_lookup;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Location name not found in database: " + display_name
        );
    }
    return iter->second;
}

// Get all location slugs in a vector
const std::vector<std::string>& LOCATION_SLUGS(){
    return LocationNameDatabase::instance().slugs;
}

// Mapping between location enum, slug, and index in menu
const std::vector<LocationItem>& LOCATION_ENUM_MAPPINGS(){
    static const std::vector<LocationItem> database{
        {Location::CENTRICO_PLAZA,                "centrico-plaza",                  0},
        {Location::GARE_DE_LUMIOSE,               "gare-de-lumiose",                 1},
        {Location::POKEMON_RESEARCH_LAB,          "pokemon-research-lab",            2},
        {Location::HOTEL_Z,                       "hotel-z",                         3},
        {Location::VERT_POKEMON_CENTER,           "vert-pokemon-center",             4},
        {Location::RACINE_CONSTRUCTION,           "racine-construction",             5},
        {Location::RESTAURANT_LE_NAH,             "restaurant-le-nah",               6},
        {Location::CAFE_CYCLONE,                  "cafe-cyclone",                    7},
        {Location::CAFE_CLASSE,                   "cafe-classe",                     8},
        {Location::CAFE_INTROVERSION,             "cafe-introversion",               9},
        {Location::NOUVEAU_CAFE,                  "nouveau-cafe",                   10},
        {Location::RUST_SYNDICATE_OFFICE,         "rust-syndicate-office",          11},
        {Location::LUMIOSE_SEWERS_1,              "lumiose-sewers-1",               12},
        {Location::BLEU_POKEMON_CENTER,           "bleu-pokemon-center",            13},
        {Location::VERNAL_POKEMON_CENTER,         "vernal-pokemon-center",          14},
        {Location::CAFE_WOOF,                     "cafe-woof",                      15},
        {Location::CAFE_SOLEIL,                   "cafe-soleil",                    16},
        {Location::SHUTTERBUG_CAFE,               "shutterbug-cafe",                17},
        {Location::NOUVEAU_CAFE_TRUCK_NO_2,       "nouveau-cafe-truck-no-2",        18},
        {Location::QUASARTICO_INC,                "quasartico-inc",                 19},
        {Location::LYSANDRE_CAFE,                 "lysandre-cafe",                  20},
        {Location::LUMIOSE_SEWERS_2,              "lumiose-sewers-2",               21},
        {Location::MAGENTA_POKEMON_CENTER,        "magenta-pokemon-center",         22},
        {Location::MAGENTA_PLAZA_POKEMON_CENTER,  "magenta-plaza-pokemon-center",   23},
        {Location::CAFE_POKEMON_AMIE,             "cafe-pokemon-amie",              24},
        {Location::CAFE_ROULEAU,                  "cafe-rouleau",                   25},
        {Location::CAFE_GALLANT,                  "cafe-gallant",                   26},
        {Location::CAFE_TRISTE,                   "cafe-triste",                    27},
        {Location::NOUVEAU_CAFE_TRUCK_NO_3,       "nouveau-cafe-truck-no-3",        28},
        {Location::HOTEL_RICHISSIME,              "hotel-richissime",               29},
        {Location::LOOKER_BUREAU,                 "looker-bureau",                  30},
        {Location::LUMIOSE_MUSEUM,                "lumiose-museum",                 31},
        {Location::ROUGE_POKEMON_CENTER,          "rouge-pokemon-center",           32},
        {Location::CENTRICO_POKEMON_CENTER,       "centrico-pokemon-center",        33},
        {Location::RESTAURANT_LE_YEAH,            "restaurant-le-yeah",             34},
        {Location::SUSHI_HIGH_ROLLER,             "sushi-high-roller",              35},
        {Location::RESTAURANT_LE_WOW,             "restaurant-le-wow",              36},
        {Location::JUSTICE_DOJO,                  "justice-dojo",                   37},
        {Location::JAUNE_POKEMON_CENTER,          "jaune-pokemon-center",           38},
        {Location::HIBERNAL_POKEMON_CENTER,       "hibernal-pokemon-center",        39},
        {Location::CAFE_ULTIMO,                   "cafe-ultimo",                    40},
        {Location::CAFE_ACTION,                   "cafe-action",                    41},
        {Location::CAFE_KIZUNA,                   "cafe-kizuna",                    42},
        {Location::CAFE_BATAILLE,                 "cafe-bataille",                  43},
        {Location::WILD_ZONE_1,                   "wild-zone-1",                    44},
        {Location::WILD_ZONE_2,                   "wild-zone-2",                    45},
        {Location::WILD_ZONE_3,                   "wild-zone-3",                    46},
        {Location::WILD_ZONE_4,                   "wild-zone-4",                    47},
        {Location::WILD_ZONE_5,                   "wild-zone-5",                    48},
        {Location::WILD_ZONE_6,                   "wild-zone-6",                    49},
        {Location::WILD_ZONE_7,                   "wild-zone-7",                    50},
        {Location::WILD_ZONE_8,                   "wild-zone-8",                    51},
        {Location::WILD_ZONE_9,                   "wild-zone-9",                    52},
        {Location::WILD_ZONE_10,                  "wild-zone-10",                   53},
        {Location::WILD_ZONE_11,                  "wild-zone-11",                   54},
        {Location::WILD_ZONE_12,                  "wild-zone-12",                   55},
        {Location::WILD_ZONE_13,                  "wild-zone-13",                   56},
        {Location::WILD_ZONE_14,                  "wild-zone-14",                   57},
        {Location::WILD_ZONE_15,                  "wild-zone-15",                   58},
        {Location::WILD_ZONE_16,                  "wild-zone-16",                   59},
        {Location::WILD_ZONE_17,                  "wild-zone-17",                   60},
        {Location::WILD_ZONE_18,                  "wild-zone-18",                   61},
        {Location::WILD_ZONE_19,                  "wild-zone-19",                   62},
        {Location::WILD_ZONE_20_NO_DISTORTION,    "wild-zone-20",                   63},
        {Location::WILD_ZONE_20_WITH_DISTORTION,  "wild-zone-20",                   63},
        {Location::HYPERSPACE_ENTRY_POINT,        "hyperspace-entry-point",         0}, // Not a valid slug currently
    };
    return database;
}

// Get the whole LocationItem given a Location enum
const LocationItem& get_location_item_from_enum(const Location location){
    const std::vector<LocationItem>& database = LOCATION_ENUM_MAPPINGS();
    for (const LocationItem& item : database){
        if (item.location == location){
            return item;
        }
    }
    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Location enumnot found in database: Enum #" + std::to_string((int)location)
    );
}

// Get the whole LocationItem given a slug
const LocationItem& get_location_item_from_slug(const std::string& slug){
    const std::vector<LocationItem>& database = LOCATION_ENUM_MAPPINGS();
    for (const LocationItem& item : database){
        if (item.slug == slug){
            return item;
        }
    }
    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Location slug not found in database: " + slug
    );
}

}
}
}