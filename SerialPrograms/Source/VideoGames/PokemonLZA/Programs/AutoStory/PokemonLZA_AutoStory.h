/*  Auto Story
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_AutoStory_H
#define PokemonAutomation_PokemonLZA_AutoStory_H

#include <memory>
#include <vector>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "VideoGames/PokemonLZA/Options/PokemonLZA_BattleAIOption.h"
#include "PokemonLZA_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class AutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoStory_Descriptor();
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class AutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    using Descriptor = AutoStory_Descriptor;
    AutoStory();
    virtual ~AutoStory();
    virtual std::string check_validity() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;

    enum class StorySection{
        TUTORIAL,
    };

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EnumDropdownOption<StorySection> STORY_SECTION;
    BooleanCheckBoxOption ENABLE_ADVANCED_MODE;
    StringSelectOption START_SEGMENT;
    StringSelectOption START_CHECKPOINT;
    StringSelectOption END_CHECKPOINT;
    StaticTextOption START_DESCRIPTION;
    StaticTextOption END_DESCRIPTION;
    StaticTextOption SETUP_NOTE;
    EnumDropdownOption<StarterChoice> STARTER;
    BattleAIOption BATTLE_AI;
    EventNotificationsOption NOTIFICATIONS;
};

const std::vector<std::unique_ptr<AutoStory_Segment>>& ALL_AUTO_STORY_SEGMENT_LIST();
const std::vector<std::unique_ptr<AutoStory_Checkpoint>>& ALL_AUTO_STORY_CHECKPOINT_LIST();

}
}
}
#endif
