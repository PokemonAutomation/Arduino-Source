/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_H
#define PokemonAutomation_PokemonSV_AutoStory_H

#include <functional>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Options/StringSelectOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_AutoStoryTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class AutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoStory_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class AutoStory : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~AutoStory();
    AutoStory();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

    void test_code(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // test the checkpoints from start to end, inclusive
    // test each checkpoints "loop" number of times
    void test_checkpoints(
        SingleSwitchProgramEnvironment& env,
        ConsoleHandle& console, 
        BotBaseContext& context,
        int start, int end, 
        int loop, int start_loop, int end_loop
    );

    void run_autostory(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    virtual void value_changed(void* object) override;

    std::string start_segment_description();
    std::string end_segment_description();

private:
    OCR::LanguageOCROption LANGUAGE;
    
    enum class StorySection{
        TUTORIAL,
        MAIN_STORY,
    };

    EnumDropdownOption<StorySection> STORY_SECTION;

    StringSelectOption STARTPOINT_TUTORIAL;
    StringSelectOption ENDPOINT_TUTORIAL;

    StringSelectOption STARTPOINT_MAINSTORY;
    StringSelectOption ENDPOINT_MAINSTORY;

    StaticTextOption MAINSTORY_NOTE;

    StaticTextOption START_DESCRIPTION;
    StaticTextOption END_DESCRIPTION;


    EnumDropdownOption<StarterChoice> STARTERCHOICE;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
    
    SectionDividerOption m_advanced_options;
    BooleanCheckBoxOption CHANGE_SETTINGS;    

    BooleanCheckBoxOption ENABLE_TEST_CHECKPOINTS;    
    SimpleIntegerOption<uint16_t> START_CHECKPOINT;
    SimpleIntegerOption<uint16_t> END_CHECKPOINT;
    SimpleIntegerOption<uint16_t> LOOP_CHECKPOINT;
    SimpleIntegerOption<uint16_t> START_LOOP;
    SimpleIntegerOption<uint16_t> END_LOOP;

    BooleanCheckBoxOption ENABLE_TEST_REALIGN;    
    EnumDropdownOption<PlayerRealignMode> REALIGN_MODE;
    SimpleIntegerOption<uint16_t> X_REALIGN;
    SimpleIntegerOption<uint16_t> Y_REALIGN;
    SimpleIntegerOption<uint16_t> REALIGN_DURATION;

    BooleanCheckBoxOption ENABLE_TEST_OVERWORLD_MOVE;    
    SimpleIntegerOption<uint16_t> FORWARD_TICKS;

    BooleanCheckBoxOption TEST_PBF_LEFT_JOYSTICK;
    SimpleIntegerOption<uint16_t> X_MOVE;
    SimpleIntegerOption<uint16_t> Y_MOVE;
    SimpleIntegerOption<uint16_t> HOLD_TICKS;
    SimpleIntegerOption<uint16_t> RELEASE_TICKS;
};





}
}
}
#endif
