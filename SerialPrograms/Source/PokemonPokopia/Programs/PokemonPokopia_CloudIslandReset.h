/*  Cloud Island Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonPokopia_CloudIslandReset_H
#define PokemonAutomation_PokemonPokopia_CloudIslandReset_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonPokopia{


class CloudIslandReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CloudIslandReset_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class CloudIslandReset : public SingleSwitchProgramInstance{
public:
    CloudIslandReset();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Starting from the overworld, face the PC and attempt to open the PC meanu
    void open_pc_from_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Starting from the overworld, navigate through the PC menu to delete the existing cloud island save and return to overworld
    void delete_cloud_island_save(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Starting from the overworld, create a new cloud island assuming none exists
    void create_cloud_island_after_delete(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Open the PC of a fresh cloud island and stop at the stamp card menu
    void open_cloud_island_pc(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Add today's stamp to the stamp collection. If full, replace a lower value one
    // Exit the stamp menu when done
    // Return true if all stamps are Mew
    bool add_todays_stamp(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Starting from the overworld on a cloud Island, return home
    void leave_cloud_island(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    DeferredStopButtonOption STOP_AFTER_CURRENT;
    BooleanCheckBoxOption IN_PALETTE_TOWN;
    SimpleIntegerOption<uint32_t> NUM_RESETS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
