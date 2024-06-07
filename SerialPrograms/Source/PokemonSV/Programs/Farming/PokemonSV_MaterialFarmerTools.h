/*  Material Farmer - Happiny dust
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_MaterialFarmerTools_H
#define PokemonAutomation_PokemonSV_MaterialFarmerTools_H

#include <functional>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"

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

class MaterialFarmerOptions : public GroupOption{
public:
    MaterialFarmerOptions()
        : MaterialFarmerOptions(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
    {}
    
    MaterialFarmerOptions(
        OCR::LanguageOCROption& language_option, GoHomeWhenDoneOption& go_home_when_done_option,
        EventNotificationOption& notif_status_update_option, EventNotificationOption& notif_program_finish_option, 
        EventNotificationOption& notif_error_recoverable_option, EventNotificationOption& notif_error_fatal_option
    )
        : MaterialFarmerOptions(&language_option, &go_home_when_done_option,
        &notif_status_update_option, &notif_program_finish_option,
        &notif_error_recoverable_option, &notif_error_fatal_option
        )
    {}
private:
    std::unique_ptr<OCR::LanguageOCROption> m_language_owner;
    std::unique_ptr<GoHomeWhenDoneOption> m_go_home_when_done_owner;
    std::unique_ptr<EventNotificationOption> m_notif_status_update_owner;
    std::unique_ptr<EventNotificationOption> m_notif_program_finish_owner;
    std::unique_ptr<EventNotificationOption> m_notif_error_recoverable_owner;
    std::unique_ptr<EventNotificationOption> m_notif_error_fatal_owner;

public:

    BooleanCheckBoxOption SAVE_GAME_BEFORE_SANDWICH;

    SimpleIntegerOption<uint16_t> NUM_SANDWICH_ROUNDS;
    
    OCR::LanguageOCROption& LANGUAGE;

    SandwichMakerOption SANDWICH_OPTIONS;

    GoHomeWhenDoneOption& GO_HOME_WHEN_DONE;

    FloatingPointOption AUTO_HEAL_PERCENT;

    // Debug options
    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
    BooleanCheckBoxOption SKIP_WARP_TO_POKECENTER;
    BooleanCheckBoxOption SKIP_SANDWICH;
    SimpleIntegerOption<uint16_t> TIME_PER_SANDWICH;
    SimpleIntegerOption<uint16_t> NUM_FORWARD_MOVES_PER_LETS_GO_ITERATION;

    EventNotificationOption& NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption& NOTIFICATION_PROGRAM_FINISH;
    EventNotificationOption& NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption& NOTIFICATION_ERROR_FATAL;
    EventNotificationsOption NOTIFICATIONS;

private:
    MaterialFarmerOptions(
        OCR::LanguageOCROption* language_option, GoHomeWhenDoneOption* go_home_when_done_option,
        EventNotificationOption* notif_status_update_option, EventNotificationOption* notif_program_finish_option, 
        EventNotificationOption* notif_error_recoverable_option, EventNotificationOption* notif_error_fatal_option
    );
};



void run_material_farmer(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    MaterialFarmerOptions& options, MaterialFarmerStats& stats);

void run_one_sandwich_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    LetsGoEncounterBotTracker& encounter_tracker, MaterialFarmerOptions& options);

void move_to_start_position_for_letsgo0(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void move_to_start_position_for_letsgo1(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void lets_go_movement0(BotBaseContext& context);

void lets_go_movement1(BotBaseContext& context);

bool is_sandwich_expired(WallClock last_sandwich_time, std::chrono::minutes minutes_per_sandwich);

void run_lets_go_iteration(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    LetsGoEncounterBotTracker& encounter_tracker, int num_forward_moves_per_lets_go_iteration
);

void run_from_battles_and_back_to_pokecenter(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    std::function<void(SingleSwitchProgramEnvironment& env, BotBaseContext& context)>&& action);



void move_from_material_farming_to_item_printer(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void fly_from_paldea_to_blueberry_entrance(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void move_from_blueberry_entrance_to_league_club(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void move_from_league_club_entrance_to_item_printer(SingleSwitchProgramEnvironment& env, BotBaseContext& context);



void move_from_item_printer_to_material_farming(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void move_from_item_printer_to_blueberry_entrance(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

void fly_from_blueberry_to_north_province_3(SingleSwitchProgramEnvironment& env, BotBaseContext& context);



}
}
}
#endif
