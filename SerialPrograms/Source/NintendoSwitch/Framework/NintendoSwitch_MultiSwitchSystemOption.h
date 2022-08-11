/*  Multi-Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomationn_NintendoSwitch_MultiSwitchSystemOption_H
#define PokemonAutomationn_NintendoSwitch_MultiSwitchSystemOption_H

#include <memory>
#include <vector>
#include "NintendoSwitch_SwitchSetup.h"
#include "NintendoSwitch_SwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchSystemWidget;

class MultiSwitchSystemOption : public SwitchSetupFactory{
public:
    static const size_t MAX_SWITCHES = 4;

public:
    MultiSwitchSystemOption(
        PABotBaseLevel min_pabotbase,
        FeedbackType feedback, bool allow_commands_while_running,
        size_t min_switches,
        size_t max_switches,
        size_t switches
    );
    MultiSwitchSystemOption(
        PABotBaseLevel min_pabotbase,
        FeedbackType feedback, bool allow_commands_while_running,
        size_t min_switches,
        size_t max_switches,
        const JsonValue& json
    );
    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

    void resize(size_t count);

public:
    size_t min_switches() const{ return m_min_switches; }
    size_t max_switches() const{ return m_max_switches; }

    size_t count() const{ return m_active_switches; }
    SwitchSystemOption& operator[](size_t index){ return *m_switches[index]; }


public:
    SwitchSetupWidget* make_ui(QWidget& parent, Logger& logger, uint64_t program_id) override;


private:
    friend class MultiSwitchSystemWidget;
    const size_t m_min_switches;
    const size_t m_max_switches;
    size_t m_active_switches;
    std::vector<std::unique_ptr<SwitchSystemOption>> m_switches;
};




}
}
#endif
