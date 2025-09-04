/*  Multi-Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This class represents the serializable state of a set of Switch.
 *  consoles. Specifically, it holds a SwitchSystemOption for each of the
 *  consoles it represents.
 *
 *  This class maintains no runtime state or UI and is not thread-safe.
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_MultiSwitchSystemOption_H
#define PokemonAutomationn_NintendoSwitch_MultiSwitchSystemOption_H

#include <memory>
#include <vector>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Panels/ProgramDescriptor.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemWidget;

class MultiSwitchSystemOption{
public:
    static const size_t MAX_SWITCHES = 4;

public:
    MultiSwitchSystemOption(
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        size_t min_switches,
        size_t max_switches,
        size_t switches
    );
    MultiSwitchSystemOption(
        FeedbackType feedback,
        AllowCommandsWhenRunning allow_commands_while_running,
        size_t min_switches,
        size_t max_switches,
        const JsonValue& json
    );
    void load_json(const JsonValue& json);
    JsonValue to_json() const;

    void resize(size_t count);

public:
    size_t min_switches() const{ return m_min_switches; }
    size_t max_switches() const{ return m_max_switches; }

    size_t count() const{ return m_active_switches; }
    SwitchSystemOption& operator[](size_t index){ return *m_switches[index]; }


private:
    friend class MultiSwitchSystemWidget;

    const bool m_allow_commands_while_running;

    const size_t m_min_switches;
    const size_t m_max_switches;
    size_t m_active_switches;
    std::vector<std::unique_ptr<SwitchSystemOption>> m_switches;
};




}
}
#endif
