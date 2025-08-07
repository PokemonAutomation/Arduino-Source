/*  Combee Finder
 *
 *  From: https://github.com/PokemonAutomation/
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
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void run_iteration(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        bool fresh_from_reset
    );
    void grouped_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void check_tree_no_stop(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool check_tree(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool handle_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void disable_shiny_sound(ProControllerContext& context);
    void enable_shiny_sound(ProControllerContext& context);

private:
    class RunRoute;

    std::atomic<bool> m_enable_shiny_sound{true};

    OCR::LanguageOCROption LANGUAGE;
    OverworldShinyDetectedActionOption SHINY_DETECTED_ENROUTE;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
};

}
}
}
#endif
