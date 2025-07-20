/*  Video Pipeline Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoPipelineOptions_H
#define PokemonAutomation_VideoPipeline_VideoPipelineOptions_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Backends/CameraImplementations.h"

namespace PokemonAutomation{


class VideoPipelineOptions : public GroupOption{
public:
    VideoPipelineOptions()
        : GroupOption(
            "Video Pipeline",
            LockMode::LOCK_WHILE_RUNNING,
            GroupOption::EnableMode::ALWAYS_ENABLED, true
        )
#if QT_VERSION_MAJOR == 5
        , ENABLE_FRAME_SCREENSHOTS(
            "<b>Enable Frame Screenshots:</b><br>"
            "Attempt to use QVideoProbe and QVideoFrame for screenshots.",
            LockMode::UNLOCK_WHILE_RUNNING,
            true
        )
#endif
        , AUTO_RESET_SECONDS(
            "<b>Video Auto-Reset:</b><br>"
            "Attempt to reset the video if this many seconds has elapsed since the last video frame (in order to fix issues with RDP disconnection, etc).<br>"
            "This option is not supported by all video frameworks.",
            LockMode::UNLOCK_WHILE_RUNNING,
            5
        )
    {
        PA_ADD_OPTION(VIDEO_BACKEND);
#if QT_VERSION_MAJOR == 5
        PA_ADD_OPTION(ENABLE_FRAME_SCREENSHOTS);
#endif

        PA_ADD_OPTION(AUTO_RESET_SECONDS);
    }

public:
    VideoBackendOption VIDEO_BACKEND;
#if QT_VERSION_MAJOR == 5
    BooleanCheckBoxOption ENABLE_FRAME_SCREENSHOTS;
#endif

    SimpleIntegerOption<uint8_t> AUTO_RESET_SECONDS;
};



}
#endif
