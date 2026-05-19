#pragma once

#include <memory>
#include <vector>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

class BDSPAutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BDSPAutoStory_Descriptor();

    virtual std::unique_ptr<SingleSwitchProgramInstance> make_instance() const override;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class BDSPAutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~BDSPAutoStory();
    BDSPAutoStory();

    virtual void program(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
        ) override;


private:
    virtual void on_config_value_changed(void* object) override;
    void update_ui_state();
private:
    BooleanCheckBoxOption ENABLE_ADVANCED_MODE;

    StringSelectOption START_CHECKPOINT_TUTORIAL;
    StringSelectOption END_CHECKPOINT_TUTORIAL;
    StaticTextOption START_DESCRIPTION_SEGMENT;
    StaticTextOption END_DESCRIPTION_SEGMENT;

    StaticTextOption START_DESCRIPTION_CHECKPOINT;
    StaticTextOption END_DESCRIPTION_CHECKPOINT;

    StringSelectOption START_CHECKPOINT_MAINSTORY;
    StringSelectOption END_CHECKPOINT_MAINSTORY;
    StringSelectOption START_SEGMENT;
    StringSelectOption END_SEGMENT;

    EnumDropdownOption<StarterChoice>      STARTERCHOICE;
    BooleanCheckBoxOption                  STARTER_SHINY;
    BooleanCheckBoxOption                  CATCH_LEGENDARY;
    EnumDropdownOption<HelpMode>           HELP_MODE;
    EnumDropdownOption<OutsideHelpPokemon> OUTSIDE_HELP_POKEMON;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_1;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_2;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_3;
    EnumDropdownOption<TorterraMove>       OUTSIDE_HELP_ATTACK_4;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
