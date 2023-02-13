/*  Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AreaZeroPlatform_H
#define PokemonAutomation_PokemonSV_AreaZeroPlatform_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{



class PlatformResetSettings : public GroupOption, private ConfigOption::Listener{
public:
    ~PlatformResetSettings();
    PlatformResetSettings();

private:
    virtual void value_changed() override;

public:
    StaticTextOption m_description;
    StaticTextOption m_sliding_window;
    SimpleIntegerOption<uint16_t> WINDOW_IN_MINUTES;
    SimpleIntegerOption<uint16_t> KILLS_IN_WINDOW;
    SimpleIntegerOption<uint16_t> ENCOUNTERS_IN_WINDOW;
//    SimpleIntegerOption<uint16_t> RESET_DURATION_MINUTES;
};




class NavigatePlatformSettings : public GroupOption{
public:
    NavigatePlatformSettings();

public:
    StaticTextOption m_description;
    SimpleIntegerOption<uint16_t> STATION_ARRIVE_PAUSE_SECONDS;
    TimeExpressionOption<uint16_t> MIDAIR_PAUSE_TIME;
};


void inside_zero_gate_to_platform(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    NavigatePlatformSettings& settings
);


bool read_platform_center(
    double& x, double& y,
    const ProgramInfo& info, ConsoleHandle& console
);




}
}
}
#endif
