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