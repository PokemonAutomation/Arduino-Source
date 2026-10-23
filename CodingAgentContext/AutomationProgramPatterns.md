# Automation Program Patterns

## Basic Program Structure (for Single-Switch Programs)

### Required Files
```
GameName_ProgramName.h      // Header with class declarations
GameName_ProgramName.cpp    // Implementation
```
Example game names: `PokemonLA`, `PokemonSwSh`, `PokemonSV`, `PokemonBDSP`

### Core Classes
```cpp
class ProgramName_Descriptor : public SingleSwitchProgramDescriptor
class ProgramName : public SingleSwitchProgramInstance
```

## Essential Includes

### Header (.h)
```cpp
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
```

### Implementation (.cpp)
```cpp
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
```

## Example Program Options

### Common Option Types
```cpp
SimpleIntegerOption<uint16_t> NUM_BOXES(
    "<b>Number of Boxes:</b>",
    LockMode::LOCK_WHILE_RUNNING,
    5, 1, 30  // default, min, max
);

BooleanCheckBoxOption ENABLE_FEATURE(
    "<b>Enable Feature:</b>",
    LockMode::LOCK_WHILE_RUNNING,
    true
);
```

### Register Options
```cpp
PA_ADD_OPTION(NUM_BOXES);
PA_ADD_OPTION(ENABLE_FEATURE);
```

## Notifications

### Declaration (in .h)
```cpp
EventNotificationOption NOTIFICATION_STATUS_UPDATE;
EventNotificationsOption NOTIFICATIONS;
```

### Initialization (in constructor)
```cpp
NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
, NOTIFICATIONS({
    &NOTIFICATION_STATUS_UPDATE,
    &NOTIFICATION_PROGRAM_FINISH,
    &NOTIFICATION_ERROR_FATAL,
})
```

### Sending Notifications
```cpp
send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
send_program_fatal_error_notification(env, NOTIFICATION_ERROR_FATAL, "Error message");
```

## Controller Input

### Button Presses
```cpp
pbf_press_button(context, BUTTON_A, 10, 100);  // button, hold_time, release_time
pbf_press_dpad(context, DPAD_DOWN, 20, 50);
context.wait_for_all_requests();  // wait for button to finish execution
```

### Delays
```cpp
context.wait_for(std::chrono::milliseconds(1000));
```

## Main Program Function Pattern

### Simplest Program
```cpp
void ProgramName::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // button presses and inference routines
}
```

## Looping Program with Ability to Recover from Minor Error

```cpp
while(true){
    try{
        // Initialization logic

        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        // Main program logic

        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    }catch(ProgramFinishedException&){ // an exception indicating the program has finished
        env.update_stats();
        send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
        throw;
    }catch(ScreenshotException& e){ // an exception indicating an error somewhere in the loop
        if(...){ // if we can recover from it
            // execute recover logic (usually is restarting the game)
            continue;
        }
        // we cannot recover from the exception. Throw the exception to the higher level to
        // trigger fatal error noticiation and cleanup.
        throw;
    }
}
```


## Program Registration

### In GameName_Panels.cpp
```cpp
// Add include
#include "Programs/Category/GameName_ProgramName.h"

// Add to make_panels()
ret.emplace_back(make_single_switch_program<ProgramName_Descriptor, ProgramName>());
```
Examples: `PokemonLA_Panels.cpp`, `PokemonSwSh_Panels.cpp`, `PokemonSV_Panels.cpp`

## Descriptor Configuration

```cpp
ProgramName_Descriptor::ProgramName_Descriptor()
    : SingleSwitchProgramDescriptor(
        "GameName:ProgramName",                // Unique ID
        STRING_POKEMON + " [GameName]", "Display Name",  // Category, Name
        "ComputerControl/blob/master/Wiki/...",     // Wiki URL
        "Brief description.",                       // Description
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,                     // or NONE
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
```
## Screen-relative matcher positioning

Repository rule: matcher targets, landmark sizes, search regions, reported offsets,
and alignment tolerances must use fractions of screen width and height, not fixed
pixel positions. Convert to pixel rectangles only where image processing requires
them, using the image dimensions. A fixed processing resolution is permitted; it
must not leak into the positioning API or movement decisions. Normalize each axis
separately, and account for aspect ratio when comparing horizontal/vertical error.
Keep this conversion within the game code; shared infrastructure changes require
prior review and approval.

## Language-independent detection

Design matchers and automation functions to work independently of the game
language unless they explicitly use OCR. Visual templates containing localized
text also create a language dependency; avoid treating them as language-neutral.
Prefer stable visual structure and screen-relative positions where reliable.
When OCR is necessary, make its language dependency explicit and support the
applicable game languages. The settings-verification migration is deferred in
Backlog.md; position detection versus OCR remains undecided.

## Usage-cap handoff to Visual Studio GitHub Copilot

Kevin requests that the final deliverable before approaching a Codex usage cap
be a copy-ready handoff prompt for Visual Studio GitHub Copilot. Use 90% consumed
in either available usage window as the default trigger, leaving room to finish
the handoff. Check available account usage at the start of substantial coding
work and before long operations. When the trigger is reached, bring authorized
work to a reviewable state, refresh CodingAgentContext/CopilotHandoff.md, include
the exact current task, pending work, files, tests, known risks, and all Kevin's
constraints, then make the handoff prompt the final deliverable of that turn.
Do not start another substantial implementation first. This is a during-work
practice, not a promise of continuous background monitoring. A hard usage cap
may prevent an additional reply, so keep the saved handoff current proactively.
The handoff is drafted for Kevin to paste; do not send messages or change
Copilot provider/account settings without separate authorization.

## Lighting setup for new locator strips

Kevin's preference: set the console clock to 13:00 before most new overworld
locator strips, preserving its current date. Set it once per contiguous strip,
not before every matcher or again for the same strip's post-battle return.
Allow the game lighting to settle after resuming. Existing locators already
validated across lighting conditions do not need retroactive clock changes.
Keep screen-relative positioning, visual confirmation, and bounded recovery;
controlled lighting does not replace those safeguards.


## Explicit checkpoint boundary contract

Every explicit checkpoint starts at the repeatable end location of its predecessor, except the very first checkpoint in Segment 00. Specify position, facing direction and relevant state (such as Repel) consistently in both descriptions. Complete any location-normalizing entry/exit before handing off; save at the next checkpoint using the existing checkpoint save/retry helper. Do not define a new checkpoint at a variable timed-movement endpoint.

Floaroma 02.01 -> 02.02: force Repel expiry/NO, travel blindly north into the central house and immediately exit, checking both door transitions. End 02.01 / start 02.02 outside that house facing south, Repel inactive. Per user preference, use blind calibrated south/east movement to the Pokemon Center, then watched entry and existing healing helpers; do not require facade/emblem locators on this local route.

## Standing Repel movement budgets (Kevin, 2026-10-18)

After successful `activate_repel(stream, context, num_icons, repel_type)`, begin a fresh cumulative movement budget for that Repel:

| repel_type | Item | Movement before forcing expiry |
| --- | --- | --- |
| 0 | Repel | 12 seconds |
| 1 | Super Repel | 22 seconds |
| 2 | Max Repel | 30 seconds |

Count actual route movement commands after activation, not wall-clock time. Exclude waits, menu navigation, dialogue and battles. At the applicable budget, use the existing forced-expiry/renewal helper at a suitable location before continuing the next route inputs. Restart the movement budget after a successful new application or renewal. Preserve the API: the third argument is `num_icons`; the fourth argument is `repel_type` (default 0).

These are user-selected standing budgets. Recording this rule does not itself implement automatic movement accounting or change gameplay code. Reuse/expand existing helpers when implementing it.


## Building-entry locators: watch transitions throughout (Kevin, 2026-10-21)

For future building-entry work, use a BlackScreenWatcher alongside the visual locator throughout the approach and alignment. If a transition starts early, cancel remaining outdoor movement/search, wait for the fade to finish, and confirm the intended interior before continuing. Also recognize an already-visible intended interior so entry is not replayed. A black screen alone does not identify the building. Reuse existing matching/transition helpers, preserve shared gates, and keep route sequencing in its owning segment.
