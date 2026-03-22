/*  Pokemon RSE Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_Settings_H
#define PokemonAutomation_PokemonRSE_Settings_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Options/BoxOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


class GameSettings : public BatchOption, private ConfigOption::Listener{
    ~GameSettings();
    GameSettings();
public:
    static GameSettings& instance();

    enum class Device{
        switch_1_2,
        rg35xx,
        custom,
    };

    SectionDividerOption m_game_device_settings;
    EnumDropdownOption<Device> DEVICE;

    BoxOption GAME_BOX;

    SectionDividerOption m_soft_reset_timings;
    MillisecondsOption SELECT_BUTTON_MASH0;
    MillisecondsOption ENTER_GAME_WAIT0;

    SectionDividerOption m_shiny_audio_settings;
    FloatingPointOption SHINY_SOUND_THRESHOLD;
    FloatingPointOption SHINY_SOUND_LOW_FREQUENCY;

private:
    virtual void on_config_value_changed(void* object) override;
};




class GameSettings_Descriptor : public PanelDescriptor{
public:
    GameSettings_Descriptor();
};


class GameSettingsPanel : public SettingsPanelInstance{
public:
    GameSettingsPanel(const GameSettings_Descriptor& descriptor);
private:
    GameSettings& settings;
};


}
}
}
#endif
