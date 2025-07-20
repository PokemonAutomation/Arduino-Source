/*  Material Farmer - Happiny dust
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MaterialFarmerTools_H
#define PokemonAutomation_PokemonSV_MaterialFarmerTools_H

#include <functional>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


struct MaterialFarmerStats : public LetsGoEncounterBotStats{
    MaterialFarmerStats()
        : m_sandwiches(m_stats["Sandwiches"])
        , m_autoheals(m_stats["Auto Heals"])
        , m_game_resets(m_stats["Game Resets"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.insert(m_display_order.begin() + 2, {"Sandwiches", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 3, {"Auto Heals", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 4, {"Game Resets", HIDDEN_IF_ZERO});
        m_display_order.insert(m_display_order.begin() + 5, {"Errors", HIDDEN_IF_ZERO});
    }
    std::atomic<uint64_t>& m_sandwiches;
    std::atomic<uint64_t>& m_autoheals;
    std::atomic<uint64_t>& m_game_resets;
    std::atomic<uint64_t>& m_errors;
};

class MaterialFarmerOptions : public GroupOption, public ConfigOption::Listener{
public:
    ~MaterialFarmerOptions();
    MaterialFarmerOptions(
        GroupOption::EnableMode enable_mode,
        OCR::LanguageOCROption* language_option,
        EventNotificationOption& notif_status_update_option,
        EventNotificationOption& notif_program_finish_option,
        EventNotificationOption& notif_error_recoverable_option,
        EventNotificationOption& notif_error_fatal_option
    );
    virtual void on_config_value_changed(void* object) override;

private:
    std::unique_ptr<OCR::LanguageOCROption> m_language_owner;

public:
    SimpleIntegerOption<uint16_t> RUN_TIME_IN_MINUTES;


//    BooleanCheckBoxOption SAVE_GAME_BEFORE_SANDWICH;
//    StaticTextOption SAVE_GAME_BEFORE_SANDWICH_STATIC_TEXT;

//    StaticTextOption NUM_SANDWICH_ROUNDS_STATIC_TEXT;
    
    OCR::LanguageOCROption& LANGUAGE;
    SandwichMakerOption SANDWICH_OPTIONS;
    FloatingPointOption AUTO_HEAL_PERCENT;

    // Debug options
    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
    BooleanCheckBoxOption SKIP_WARP_TO_POKECENTER;
//    BooleanCheckBoxOption ENABLE_SANDWICH;
    SimpleIntegerOption<uint16_t> TIME_PER_SANDWICH;
    SimpleIntegerOption<uint16_t> NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION;

    EventNotificationOption& NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption& NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption& NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption& NOTIFICATION_ERROR_FATAL;
};



void run_material_farmer(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    MaterialFarmerOptions& options,
    MaterialFarmerStats& stats
);

void check_hp(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    MaterialFarmerOptions& options,
    LetsGoHpWatcher& hp_watcher,
    MaterialFarmerStats& stats
);

WallClock make_sandwich_material_farm(
    ProgramEnvironment& env, 
    VideoStream& stream,
    ProControllerContext& context, 
    MaterialFarmerOptions& options,
    MaterialFarmerStats& stats
);        

void move_to_start_position_for_letsgo0(VideoStream& stream, ProControllerContext& context);

void move_to_start_position_for_letsgo1(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void lets_go_movement0(ProControllerContext& context);

void lets_go_movement1(ProControllerContext& context);

std::chrono::minutes minutes_remaining(WallClock start_time, std::chrono::minutes minutes_duration);

void run_lets_go_iteration(
    VideoStream& stream,
    ProControllerContext& context,
    LetsGoEncounterBotTracker& encounter_tracker,
    int num_forward_moves_per_lets_go_iteration
);

void run_from_battles_and_back_to_pokecenter(
    ProgramEnvironment& env, 
    VideoStream& stream,
    ProControllerContext& context,
    MaterialFarmerStats& stats,
    std::function<
        void(ProgramEnvironment& env,
        VideoStream& stream,
        ProControllerContext& context)
    >&& action
);



void move_from_material_farming_to_item_printer(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void fly_from_paldea_to_blueberry_entrance(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void move_from_blueberry_entrance_to_league_club(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void move_from_league_club_entrance_to_item_printer(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);



void move_from_item_printer_to_material_farming(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void move_from_item_printer_to_blueberry_entrance(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

void fly_from_blueberry_to_north_province_3(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);



}
}
}
#endif
