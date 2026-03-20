/*  Pokemon RSE Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Pokemon_Strings.h"

#include "PokemonRSE_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::~GameSettings(){
    DEVICE.remove_listener(*this);
    GAME_BOX.remove_listener(*this);
    GAME_BOX.X.remove_listener(*this);
    GAME_BOX.Y.remove_listener(*this);
    GAME_BOX.WIDTH.remove_listener(*this);
    GAME_BOX.HEIGHT.remove_listener(*this);
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_game_device_settings("<font size=4><b>Game Device settings:</b></font>")
    , DEVICE(
        "<b>Device:</b><br>Select the device the game is running on. "
        "Refer to the documentation for specific setups.",
        {
            {Device::switch_1_2,            "switch_1_2",           "Nintendo Switch 1 and 2"},
            {Device::rg35xx,                "rg35xx",               "RG35XX"},
            //{Device::custom,                "custom",               "Custom"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Device::switch_1_2
    )
    , GAME_BOX(
        "Game Box: The part of the screen containing the actual video feed.",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED,
        true,
        {0.09375, 0.00462963, 0.8125, 0.962963}
    )
    , m_soft_reset_timings("<font size=4><b>Soft Reset Timings:</b></font>")
    , SELECT_BUTTON_MASH0(
        "<b>Select Button Mash:</b><br>Mash select for this long after a soft reset to get to Press Start.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , ENTER_GAME_WAIT0(
        "<b>Enter Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , m_shiny_audio_settings("<font size=4><b>Shiny Audio Settings:</b></font>")
    , SHINY_SOUND_THRESHOLD(
        "<b>Shiny Sound Threshold:</b><br>Maximum error coefficient to trigger a shiny detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.80, 0, 1.0
    )
    , SHINY_SOUND_LOW_FREQUENCY(
        "<b>Shiny Sound Low Frequency (Hz):</b><br>High pass filter frequency for shiny sound.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 0, 48000
    )
{
    PA_ADD_STATIC(m_game_device_settings);
    PA_ADD_OPTION(DEVICE);
    PA_ADD_STATIC(GAME_BOX);
    PA_ADD_STATIC(m_soft_reset_timings);
    PA_ADD_OPTION(SELECT_BUTTON_MASH0);
    PA_ADD_OPTION(ENTER_GAME_WAIT0);
    PA_ADD_STATIC(m_shiny_audio_settings);
    PA_ADD_OPTION(SHINY_SOUND_THRESHOLD);
    PA_ADD_OPTION(SHINY_SOUND_LOW_FREQUENCY);

    GameSettings::on_config_value_changed(this);
    DEVICE.add_listener(*this);
    GAME_BOX.add_listener(*this);
    GAME_BOX.X.add_listener(*this);
    GAME_BOX.Y.add_listener(*this);
    GAME_BOX.WIDTH.add_listener(*this);
    GAME_BOX.HEIGHT.add_listener(*this);
}

void GameSettings::on_config_value_changed(void* object){
    switch (DEVICE){
    case Device::switch_1_2:
        GAME_BOX.X.set(0.09375);
        GAME_BOX.Y.set(0.00462963);
        GAME_BOX.WIDTH.set(0.8125);
        GAME_BOX.HEIGHT.set(0.962963);
        GAME_BOX.set_visibility(ConfigOptionState::DISABLED);
        break;
    case Device::rg35xx:
        GAME_BOX.X.set(0.125);
        GAME_BOX.Y.set(0.0564814814814815);
        GAME_BOX.WIDTH.set(0.7494791666666667);
        GAME_BOX.HEIGHT.set(0.8861111111111111);
        GAME_BOX.set_visibility(ConfigOptionState::DISABLED);
        break;
    case Device::custom:
        GAME_BOX.set_visibility(ConfigOptionState::ENABLED);
        break;
    default:
        GAME_BOX.X.set(0.09375);
        GAME_BOX.Y.set(0.00462963);
        GAME_BOX.WIDTH.set(0.8125);
        GAME_BOX.HEIGHT.set(0.962963);
        GAME_BOX.set_visibility(ConfigOptionState::ENABLED);
        break;
    }
}



GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonRSE:GlobalSettings",
        STRING_POKEMON + " RSE", "Game Settings",
        "Programs/PokemonRSE/RSESettings.html",
        "Global " + STRING_POKEMON + " Ruby, Sapphire, and Emerald Settings"
    )
{}



GameSettingsPanel::GameSettingsPanel(const GameSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
    , settings(GameSettings::instance())
{
    PA_ADD_OPTION(settings);
}





}
}
}






