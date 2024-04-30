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
#include "PokemonSV_MaterialFarmerTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class LetsGoEncounterBotTracker;


class MaterialFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MaterialFarmer_Descriptor();

    struct Stats : public LetsGoEncounterBotStats{
        Stats()
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
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MaterialFarmer : public SingleSwitchProgramInstance{
public:
    MaterialFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
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

};





}
}
}
#endif
