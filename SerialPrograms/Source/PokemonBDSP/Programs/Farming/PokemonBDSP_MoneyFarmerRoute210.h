/*  Money Farmer (Route 210)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MoneyFarmerRoute210_H
#define PokemonAutomation_PokemonBDSP_MoneyFarmerRoute210_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"
#include "PokemonBDSP/Options/PokemonBDSP_LearnMove.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MoneyFarmerRoute210_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MoneyFarmerRoute210_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MoneyFarmerRoute210 : public SingleSwitchProgramInstance{
public:
    MoneyFarmerRoute210();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Run the battle loop. Return true if the program should stop.
    bool battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t pp0[4], uint8_t pp1[4]);
    // From the bottom row of the Ace Trainer pair, heal Pokemon and return.
    // Return true if VS Seeker needs charging.
    bool heal_after_battle_and_return(
        SingleSwitchProgramEnvironment& env,
        VideoStream& stream, ProControllerContext& context,
        uint8_t pp0[4], uint8_t pp1[4]
    );
    // Starting in front of the Celestic Town Pokecenter, heal and return
    // to the Ace Trainer pair.
    void heal_at_center_and_return(Logger& logger, ProControllerContext& context, uint8_t pp0[4], uint8_t pp1[4]);
    // Fly from the Ace Trainer pair to Hearthome Pokecenter, heal and return.
    void fly_to_center_heal_and_return(
        Logger& logger, ProControllerContext& context,
        uint8_t pp0[4],uint8_t pp1[4]
    );
    // Move around to charge VS Seeker.
    void charge_vs_seeker(VideoStream& stream);

    static bool has_pp(uint8_t pp0[4], uint8_t pp1[4]);

private:
    enum class StartLocation{
        CelesticTown,
        AceTrainerPair,
    };
    enum class HealMethod{
        CelesticTown,
        GlobalRoom,
    };

    ShortcutDirectionOption SHORTCUT;

    EnumDropdownOption<StartLocation> START_LOCATION;
    EnumDropdownOption<HealMethod> HEALING_METHOD;
    OnLearnMoveOption ON_LEARN_MOVE;

    SimpleIntegerOption<uint8_t> MON0_MOVE1_PP;
    SimpleIntegerOption<uint8_t> MON0_MOVE2_PP;
    SimpleIntegerOption<uint8_t> MON0_MOVE3_PP;
    SimpleIntegerOption<uint8_t> MON0_MOVE4_PP;

    SimpleIntegerOption<uint8_t> MON1_MOVE1_PP;
    SimpleIntegerOption<uint8_t> MON1_MOVE2_PP;
    SimpleIntegerOption<uint8_t> MON1_MOVE3_PP;
    SimpleIntegerOption<uint8_t> MON1_MOVE4_PP;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
