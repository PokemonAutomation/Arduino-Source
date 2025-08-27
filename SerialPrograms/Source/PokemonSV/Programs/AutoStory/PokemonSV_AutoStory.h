/*  Autostory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoStory_H
#define PokemonAutomation_PokemonSV_AutoStory_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    void test_code(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // test the checkpoints from start to end, inclusive
    // test each checkpoints "loop" number of times
    void test_checkpoints(
        SingleSwitchProgramEnvironment& env,
        VideoStream& stream,
        ProControllerContext& context,
        int start, int end, 
        int loop, int start_loop, int end_loop
    );

    size_t get_start_segment_index();
    size_t get_end_segment_index();

    void run_autostory(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    virtual void on_config_value_changed(void* object) override;

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

    StaticTextOption SETUP_NOTE;
    StaticTextOption MAINSTORY_NOTE;

    StaticTextOption START_DESCRIPTION;
    StaticTextOption END_DESCRIPTION;


    EnumDropdownOption<StarterChoice> STARTERCHOICE;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
    
    SectionDividerOption m_advanced_options;
    SectionDividerOption m_advanced_options_end;
    BooleanCheckBoxOption CHANGE_SETTINGS;    

    BooleanCheckBoxOption ENABLE_TEST_CHECKPOINTS;    
    SimpleIntegerOption<uint16_t> START_CHECKPOINT;
    SimpleIntegerOption<uint16_t> END_CHECKPOINT;
    SimpleIntegerOption<uint16_t> LOOP_CHECKPOINT;
    SimpleIntegerOption<uint16_t> START_LOOP;
    SimpleIntegerOption<uint16_t> END_LOOP;

    BooleanCheckBoxOption ENABLE_TEST_REALIGN;    
    EnumDropdownOption<PlayerRealignMode> REALIGN_MODE;
    SimpleIntegerOption<uint8_t> X_REALIGN;
    SimpleIntegerOption<uint8_t> Y_REALIGN;
    SimpleIntegerOption<uint16_t> REALIGN_DURATION;

    BooleanCheckBoxOption ENABLE_MISC_TEST;    
    SimpleIntegerOption<uint16_t> FORWARD_TICKS;

    BooleanCheckBoxOption TEST_PBF_LEFT_JOYSTICK;
    SimpleIntegerOption<uint8_t> X_MOVE;
    SimpleIntegerOption<uint8_t> Y_MOVE;
    SimpleIntegerOption<uint16_t> HOLD_TICKS;
    SimpleIntegerOption<uint16_t> RELEASE_TICKS;

    BooleanCheckBoxOption TEST_PBF_LEFT_JOYSTICK2;
    SimpleIntegerOption<uint8_t> X_MOVE2;
    SimpleIntegerOption<uint8_t> Y_MOVE2;
    SimpleIntegerOption<uint16_t> HOLD_TICKS2;
    SimpleIntegerOption<uint16_t> RELEASE_TICKS2;    

    BooleanCheckBoxOption TEST_CURRENT_DIRECTION;
    BooleanCheckBoxOption TEST_CHANGE_DIRECTION;
    FloatingPointOption DIR_RADIANS;
};

const std::vector<std::unique_ptr<AutoStory_Segment>>& ALL_AUTO_STORY_SEGMENT_LIST();



}
}
}
#endif
