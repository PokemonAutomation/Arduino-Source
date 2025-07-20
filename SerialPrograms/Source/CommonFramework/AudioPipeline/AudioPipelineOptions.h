/*  Audio Pipeline Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioPipelineOptions_H
#define PokemonAutomation_AudioPipeline_AudioPipelineOptions_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "CommonFramework/GlobalSettingsPanel.h"

namespace PokemonAutomation{


class AudioPipelineOptions : public GroupOption{
public:
    AudioPipelineOptions()
        : GroupOption(
            "Audio Pipeline",
            LockMode::LOCK_WHILE_RUNNING,
            GroupOption::EnableMode::ALWAYS_ENABLED, true
        )
        , FILE_VOLUME_SCALE(
            "<b>Audio File Input Volume Scale:</b><br>"
            "Multiply audio file playback by this factor. (This is linear scale. So each factor of 10 is 20dB.)",
            LockMode::UNLOCK_WHILE_RUNNING,
            0.31622776601683793320, //  -10dB
            -10000, 10000
        )
        , DEVICE_VOLUME_SCALE(
            "<b>Audio Device Input Volume Scale:</b><br>"
            "Multiply audio device input by this factor. (This is linear scale. So each factor of 10 is 20dB.)",
            LockMode::UNLOCK_WHILE_RUNNING,
            1.0, -10000, 10000
        )
        , SHOW_ALL_DEVICES(
            "<b>Show all Audio Devices:</b><br>"
            "Show all audio devices - including duplicates.",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
        , SHOW_RECORD_FREQUENCIES(
            "<b>Show Record Frequencies:</b><br>"
            "Show option to record audio frequencies.",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
        , AUTO_RESET_SECONDS(
            "<b>Audio Auto-Reset:</b><br>"
            "Attempt to reset the audio if this many seconds has elapsed since the last audio frame (in order to fix issues with RDP disconnection, etc).",
            LockMode::UNLOCK_WHILE_RUNNING,
            5
        )
    {
        PA_ADD_OPTION(FILE_VOLUME_SCALE);
        PA_ADD_OPTION(DEVICE_VOLUME_SCALE);
        PA_ADD_OPTION(SHOW_ALL_DEVICES);
        if (PreloadSettings::instance().DEVELOPER_MODE){
            PA_ADD_OPTION(SHOW_RECORD_FREQUENCIES);
        }
        PA_ADD_OPTION(AUTO_RESET_SECONDS);
    }

public:
    FloatingPointOption FILE_VOLUME_SCALE;
    FloatingPointOption DEVICE_VOLUME_SCALE;
    BooleanCheckBoxOption SHOW_ALL_DEVICES;
    BooleanCheckBoxOption SHOW_RECORD_FREQUENCIES;
    SimpleIntegerOption<uint8_t> AUTO_RESET_SECONDS;
};



}
#endif
