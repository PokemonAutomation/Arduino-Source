/*  Friend Delete
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_MenuStabilityTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



MenuStabilityTester_Descriptor::MenuStabilityTester_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:MenuStabilityTester",
        "Nintendo Switch", "Menu Stability Tester",
        "",
        "Test the speed and stability of various fast menu movements.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

MenuStabilityTester::~MenuStabilityTester(){
    TEST_TYPE.remove_listener(*this);
}
MenuStabilityTester::MenuStabilityTester()
    : TEST_TYPE(
        std::move("Console Type:"),
        {
           {TestType::Vertical,             "vertical",     "Vertical Up-Down"},
           {TestType::Horizontal,           "horizontal",   "Horizontal Side-to-Side"},
           {TestType::SimultaneousScrollA,  "scroll-A",     "Simultaneous Scroll + A"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        TestType::Vertical
    )
    , VERTICAL_RANGE(
        "<b>Vertical Scroll Range:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        20
    )
    , HORIZONTAL_RANGE(
        "<b>Horizontal Scroll Range:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        4
    )
    , PAUSE_BEFORE_UTURN(
        "<b>Pause Before Turning Around:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , DELAY_TO_NEXT(
        "<b>Delay to Next Button:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "24 ms"
    )
    , HOLD_DURATION(
        "<b>Hold Duration:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "48 ms"
    )
    , COOLDOWN(
        "<b>Cooldown:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "24 ms"
    )
{
    PA_ADD_OPTION(TEST_TYPE);

    PA_ADD_OPTION(VERTICAL_RANGE);
    PA_ADD_OPTION(HORIZONTAL_RANGE);
    PA_ADD_OPTION(PAUSE_BEFORE_UTURN);

    PA_ADD_OPTION(DELAY_TO_NEXT);
    PA_ADD_OPTION(HOLD_DURATION);
    PA_ADD_OPTION(COOLDOWN);

    MenuStabilityTester::on_config_value_changed(this);

    TEST_TYPE.add_listener(*this);
}
void MenuStabilityTester::on_config_value_changed(void* object){
    switch (TEST_TYPE){
    case TestType::Vertical:
        VERTICAL_RANGE.set_visibility(ConfigOptionState::ENABLED);
        HORIZONTAL_RANGE.set_visibility(ConfigOptionState::HIDDEN);
        PAUSE_BEFORE_UTURN.set_visibility(ConfigOptionState::ENABLED);
        break;
    case TestType::Horizontal:
        VERTICAL_RANGE.set_visibility(ConfigOptionState::HIDDEN);
        HORIZONTAL_RANGE.set_visibility(ConfigOptionState::ENABLED);
        PAUSE_BEFORE_UTURN.set_visibility(ConfigOptionState::ENABLED);
        break;
    case TestType::SimultaneousScrollA:
        VERTICAL_RANGE.set_visibility(ConfigOptionState::HIDDEN);
        HORIZONTAL_RANGE.set_visibility(ConfigOptionState::HIDDEN);
        PAUSE_BEFORE_UTURN.set_visibility(ConfigOptionState::HIDDEN);
        break;
    }
}




void MenuStabilityTester::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    switch (TEST_TYPE){
    case TestType::Vertical:
        while (true){
            for (size_t c = 0; c < VERTICAL_RANGE; c++){
                ssf_issue_scroll(context, DPAD_DOWN, DELAY_TO_NEXT, HOLD_DURATION, COOLDOWN);
            }
            if (PAUSE_BEFORE_UTURN){
                ssf_do_nothing(context, 1000ms);
            }
            for (size_t c = 0; c < VERTICAL_RANGE; c++){
                ssf_issue_scroll(context, DPAD_UP, DELAY_TO_NEXT, HOLD_DURATION, COOLDOWN);
            }
            if (PAUSE_BEFORE_UTURN){
                ssf_do_nothing(context, 1000ms);
            }
        }
        break;

    case TestType::Horizontal:
        while (true){
            for (size_t c = 0; c < HORIZONTAL_RANGE; c++){
                ssf_issue_scroll(context, DPAD_RIGHT, DELAY_TO_NEXT, HOLD_DURATION, COOLDOWN);
            }
            if (PAUSE_BEFORE_UTURN){
                ssf_do_nothing(context, 1000ms);
            }
            for (size_t c = 0; c < HORIZONTAL_RANGE; c++){
                ssf_issue_scroll(context, DPAD_LEFT, DELAY_TO_NEXT, HOLD_DURATION, COOLDOWN);
            }
            if (PAUSE_BEFORE_UTURN){
                ssf_do_nothing(context, 1000ms);
            }
        }
        break;

    case TestType::SimultaneousScrollA:
        while (true){
            ssf_press_button(context, BUTTON_A, 0ms, HOLD_DURATION, COOLDOWN);
            ssf_issue_scroll(context, DPAD_UP, DELAY_TO_NEXT, HOLD_DURATION, COOLDOWN);
            ssf_do_nothing(context, 1000ms);
        }
        break;

    }
}




















}
}
