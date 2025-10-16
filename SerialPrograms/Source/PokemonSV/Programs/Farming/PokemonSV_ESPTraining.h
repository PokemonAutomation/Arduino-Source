/*  ESP Training
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ESPTraining_H
#define PokemonAutomation_PokemonSV_ESPTraining_H

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class ESPTraining_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ESPTraining_Descriptor();

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

struct ESPTrainingStats : public StatsTracker{
    ESPTrainingStats()
        : m_emotions(m_stats["Emotions Shown"])
        , m_joy(m_stats["Joy"])
        , m_surprise(m_stats["Surprise"])
        , m_excitement(m_stats["Excitement"])
        , m_anger(m_stats["Anger"])
        , m_clears(m_stats["Times Cleared"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Emotions Shown");
        m_display_order.emplace_back("Joy");
        m_display_order.emplace_back("Surprise");
        m_display_order.emplace_back("Excitement");
        m_display_order.emplace_back("Anger");
        m_display_order.emplace_back("Times Cleared");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_emotions;
    std::atomic<uint64_t>& m_joy;
    std::atomic<uint64_t>& m_surprise;
    std::atomic<uint64_t>& m_excitement;
    std::atomic<uint64_t>& m_anger;
    std::atomic<uint64_t>& m_clears;
    std::atomic<uint64_t>& errors;
};

class ESPTraining : public SingleSwitchProgramInstance{
public:
    ESPTraining();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> ROUNDS;

    BooleanCheckBoxOption SAVE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

//Detect emotion and press the right button
//151 emotions + pauses but the game is inconsistent and sometimes displays an emotion during the transitions
//Note: can hit the wrong emotion and then the right one right after, as long as its before the timer
void run_esp_training(SingleSwitchProgramEnvironment& env, ProControllerContext& context, ESPTrainingStats& stats);

}
}
}
#endif
