/*  Donut Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutMaker_H
#define PokemonAutomation_PokemonLZA_DonutMaker_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "PokemonLZA/Options/PokemonLZA_DonutBerriesOption.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class DonutMaker_Descriptor : public SingleSwitchProgramDescriptor {
public:
    DonutMaker_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class DonutMaker : public SingleSwitchProgramInstance{
public:
    DonutMaker();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool match_powers(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::vector<uint16_t>& kept_counts);
    bool should_stop(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const std::vector<uint16_t>& kept_counts);
    void animation_to_donut(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void add_berries_and_make_donut(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void open_berry_menu_from_ansha(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool donut_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::vector<uint16_t>& kept_counts);

private:

    OCR::LanguageOCROption LANGUAGE;
    DonutBerriesTable BERRIES;
    SimpleIntegerOption<uint16_t> MAX_KEEPERS;
    FlavorPowerTable FLAVOR_POWERS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_DONUT_FOUND;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
