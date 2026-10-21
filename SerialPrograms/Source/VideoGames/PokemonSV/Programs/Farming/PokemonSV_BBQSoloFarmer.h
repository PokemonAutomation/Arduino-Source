/*  BBQ Solo Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BBQSoloFarmer_H
#define PokemonAutomation_PokemonSV_BBQSoloFarmer_H

#include <stdint.h>
#include <vector>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_BBQOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// The game actions that run_bbq_rounds() depends on. They are injected so
// that the round loop (which quests to attempt, which ones count, when to
// stop, when to save) runs in tests without a Switch: BBQSoloFarmer performs
// them on the console, and tests replace them with scripted results.
class BBQGameActions{
public:
    virtual ~BBQGameActions() = default;

    // Open the quest panel and read the quests on it.
    virtual std::vector<BBQuests> read_quests() = 0;

    // Decide which of the quests just read are worth attempting this round.
    // May reroll quests in game, so the returned list can be empty even when
    // `quests` is not; the loop then reads the panel again.
    virtual std::vector<BBQuests> process_quest_list(
        const std::vector<BBQuests>& quests, uint8_t eggs_hatched
    ) = 0;

    // Attempt one quest that is on the panel, after process_quest_list()
    // chose it. Returns true if it is no longer on the panel afterwards.
    // Increments `eggs_hatched` when the quest hatches eggs.
    virtual bool do_quest(BBQuests quest, uint8_t& eggs_hatched) = 0;

    // Progress reporting only: a quest was counted as completed. Lets stats
    // update after every quest instead of once per round.
    virtual void quest_completed() = 0;

    // A round has ended. The loop decides whether a save is due, because
    // that depends on how many quests it has counted; how to fix the date,
    // save and reset, and report the round is up to the game side.
    virtual void end_round(bool save) = 0;
};

// Run rounds of quests until NUM_QUESTS have been completed, or until
// eggs run out when OUT_OF_EGGS is Stop.
// Throws UserSetupError if the exclusions leave no blue quest to do, or if
// the panel shows nothing doable for many reads in a row.
void run_bbq_rounds(Logger& logger, const BBQOption& options, BBQGameActions& game);


class BBQSoloFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BBQSoloFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BBQSoloFarmer : public SingleSwitchProgramInstance{
public:
    using Descriptor = BBQSoloFarmer_Descriptor;
    BBQSoloFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    BBQOption BBQ_OPTIONS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif
