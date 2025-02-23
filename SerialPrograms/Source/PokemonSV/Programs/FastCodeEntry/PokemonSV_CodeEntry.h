/*  Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_CodeEntry_H
#define PokemonAutomation_PokemonSV_CodeEntry_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Programs/NintendoSwitch_KeyboardCodeEntry.h"

namespace PokemonAutomation{
    class CancellableScope;
    class ProgramEnvironment;
namespace NintendoSwitch{
    class MultiSwitchProgramEnvironment;
namespace PokemonSV{


class FastCodeEntryKeyboardLayout : public GroupOption{
public:
    FastCodeEntryKeyboardLayout();

public:
    FixedLimitVector<KeyboardLayoutOption> CONSOLE;
};


class FastCodeEntrySettingsOption : public BatchOption{
public:
    FastCodeEntrySettingsOption();

    void set_active_consoles(size_t active_consoles);

public:
    CodeEntrySkipPlusOption SKIP_PLUS;
    FastCodeEntryKeyboardLayout KEYBOARD_LAYOUTS;
};

struct FastCodeEntrySettings{
    bool skip_plus = false;
    KeyboardLayout keyboard_layout[4];

    FastCodeEntrySettings() = default;
    FastCodeEntrySettings(const FastCodeEntrySettingsOption& settings)
        : skip_plus(settings.SKIP_PLUS)
    {
        keyboard_layout[0] = settings.KEYBOARD_LAYOUTS.CONSOLE[0];
        keyboard_layout[1] = settings.KEYBOARD_LAYOUTS.CONSOLE[1];
        keyboard_layout[2] = settings.KEYBOARD_LAYOUTS.CONSOLE[2];
        keyboard_layout[3] = settings.KEYBOARD_LAYOUTS.CONSOLE[3];
    }
};



const char* normalize_code(std::string& normalized_code, const std::string& code, bool override_mode = false);

void enter_code(
    Logger& logger, ProControllerContext& context,
    KeyboardLayout keyboard_layout,
    const std::string& normalized_code, bool force_keyboard_mode,
    bool include_plus,
    bool connect_controller_press
);

const char* enter_code(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    const FastCodeEntrySettings& settings,
    const std::string& code, bool force_keyboard_mode,
    bool connect_controller_press
);





}
}
}
#endif
