/*  Wild RNG
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_WildRng_H
#define PokemonAutomation_PokemonFRLG_WildRng_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TextEditOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_RngDisplays.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class WildRng_Descriptor : public SingleSwitchProgramDescriptor{
public:
    WildRng_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class WildRng : public SingleSwitchProgramInstance{
public:
    WildRng();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:

    enum class GameVersion{
        firered,
        leafgreen
    };

    enum class EncounterType{
        grass,
        rocksmash,
        surfing,
        oldrod,
        goodrod,
        superrod
    };

    enum class GameLocation{
        altering_cave,
        berry_forest,
        bond_bridge,
        cape_brink,
        celadon_city,
        cerulean_cave_1f,
        cerulean_cave_2f,
        cerulean_cave_b1f,
        cerulean_city,
        cinnabar_island,
        digletts_cave,
        five_island,
        five_isle_meadow,
        four_island,
        fuchsia_city,
        green_path,
        icefall_cave_1f,
        icefall_cave_back_cavern,
        icefall_cave_b1f,
        icefall_cave_entrance,
        kindle_road,
        lost_cave,
        memorial_pillar,
        mt_ember_exterior,
        mt_ember_ruby_path_1f,
        mt_ember_ruby_path_b1f,
        mt_ember_ruby_path_b2f,
        mt_ember_ruby_path_b3f,
        mt_ember_summit_path_1f,
        mt_ember_summit_path_2f,
        mt_ember_summit_path_3f,
        mt_moon_1f,
        mt_moon_b1f,
        mt_moon_b2f,
        one_island,
        outcast_island,
        pallet_town,
        pattern_bush,
        pokemon_mansion_basement,
        pokemon_mansion_upper_floors,
        pokemon_tower_3f,
        pokemon_tower_4f,
        pokemon_tower_5f,
        pokemon_tower_6f,
        pokemon_tower_7f,
        power_plant,
        resort_gorgeous,
        rock_tunnel_1f,
        rock_tunnel_b1f,
        route_1,
        route_10,
        route_11,
        route_12,
        route_13,
        route_14,
        route_15,
        route_16,
        route_17,
        route_18,
        route_19,
        route_2,
        route_20,
        route_21,
        route_22,
        route_23,
        route_24,
        route_25,
        route_3,
        route_4,
        route_5,
        route_6,
        route_7,
        route_8,
        route_9,
        ruin_valley,
        safari_zone_area_1_east,
        safari_zone_area_2_north,
        safari_zone_area_3_west,
        safari_zone_entrance,
        seafoam_islands_1f,
        seafoam_islands_b1f,
        seafoam_islands_b2f,
        seafoam_islands_b3f,
        seafoam_islands_b4f,
        sevault_canyon,
        sevault_canyon_entrance,
        ss_anne,
        tanoby_ruins,
        tanoby_ruins_chambers,
        three_isle_port,
        trainer_tower,
        treasure_beach,
        vermilion_city,
        victory_road_1f,
        victory_road_2f,
        victory_road_3f,
        viridian_city,
        viridian_forest,
        water_labyrinth,
        water_path,
    };

    bool have_hit_target(SingleSwitchProgramEnvironment& env, const uint32_t& TARGET_SEED, const AdvRngState& hit);

    AdvObservedPokemon read_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::set<std::string>& SPECIES_LIST);

    bool auto_catch(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context, 
        WildRng_Descriptor::Stats& stats,
        const uint64_t& MAX_BALL_THROWS
    );

    bool use_rare_candy(
        SingleSwitchProgramEnvironment& env, 
        ProControllerContext& context,
        WildRng_Descriptor::Stats& stats,
        AdvObservedPokemon& pokemon,
        AdvRngFilters& filters,
        const BaseStats& BASE_STATS,
        bool first
    );
    
    OCR::LanguageOCROption LANGUAGE;

    EnumDropdownOption<GameVersion> GAME_VERSION;
    EnumDropdownOption<EncounterType> ENCOUNTER_TYPE;
    EnumDropdownOption<GameLocation> GAME_LOCATION;

    SimpleIntegerOption<uint64_t> MAX_RESETS;
    SimpleIntegerOption<uint64_t> MAX_RARE_CANDIES;
    SimpleIntegerOption<uint64_t> MAX_BALL_THROWS;

    RngFilterDisplay RNG_FILTERS;
    RngCalibrationDisplay RNG_CALIBRATION;

    StringOption SEED; 
    TextEditOption SEED_LIST;
    EnumDropdownOption<SeedButton> SEED_BUTTON;
    EnumDropdownOption<BlackoutButton> EXTRA_BUTTON;
    SimpleIntegerOption<uint64_t> SEED_DELAY;

    SimpleIntegerOption<uint64_t>ADVANCES;

    BooleanCheckBoxOption USE_TEACHY_TV;

    SimpleIntegerOption<uint8_t> PROFILE;

    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif
