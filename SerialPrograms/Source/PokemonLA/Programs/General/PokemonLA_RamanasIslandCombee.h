/*  Combee Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_RamanasIslandCombee_H
#define PokemonAutomation_PokemonLA_RamanasIslandCombee_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class RamanasCombeeFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RamanasCombeeFinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class RamanasCombeeFinder: public SingleSwitchProgramInstance{
public:
    RamanasCombeeFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    void run_iteration(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void grouped_path(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void check_tree_no_stop(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    bool check_tree(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    bool handle_battle(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context);
    void disable_shiny_sound(SwitchControllerContext& context);
    void enable_shiny_sound(SwitchControllerContext& context);

private:
    class RunRoute;

    std::atomic<bool> m_enable_shiny_sound{true};

    OCR::LanguageOCROption LANGUAGE;
    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
};

}
}
}
#endif
