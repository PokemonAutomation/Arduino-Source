/*  Material Farmer - Happiny dust
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_MaterialFarmer_H
#define PokemonAutomation_PokemonSV_MaterialFarmer_H

#include <functional>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class LetsGoEncounterBotTracker;


class MaterialFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MaterialFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class MaterialFarmer : public SingleSwitchProgramInstance{
public:
    MaterialFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_one_sandwich_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    
    void run_lets_go_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void move_to_start_position_for_letsgo0(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void move_to_start_position_for_letsgo1(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void lets_go_movement0(BotBaseContext& context);

    void lets_go_movement1(BotBaseContext& context);

    bool is_sandwich_expired(WallClock last_sandwich_time);

    void handle_battles_and_back_to_pokecenter(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
        std::function<void(SingleSwitchProgramEnvironment& env, BotBaseContext& context)>&& action);

    BooleanCheckBoxOption SAVE_GAME_BEFORE_SANDWICH;

    SimpleIntegerOption<uint16_t> NUM_SANDWICH_ROUNDS;
    
    OCR::LanguageOCROption LANGUAGE;

    SandwichMakerOption SANDWICH_OPTIONS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    FloatingPointOption AUTO_HEAL_PERCENT;

    // Debug options
    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
    BooleanCheckBoxOption SKIP_WARP_TO_POKECENTER;
    BooleanCheckBoxOption SKIP_SANDWICH;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    LetsGoEncounterBotTracker* m_encounter_tracker;

};





}
}
}
#endif
