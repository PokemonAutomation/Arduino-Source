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
            GroupOption::EnableMode::ALWAYS_ENABLED,
            true,
            false
        )
        , SHOW_ALL_FPS(
            "<b>Show all FPS Options:</b><br>"
            "Show all video format/FPS combinations. This is hidden by default since Qt usually ignores the FPS.",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
        , AUTO_RESET_SECONDS(
            "<b>Video Auto-Reset:</b><br>"
            "Attempt to reset the video if this many seconds has elapsed since the last video frame (in order to fix issues with RDP disconnection, etc).<br>"
            "This option is not supported by all video frameworks. Set to a value of 0 to disable Auto-Reset.",
            LockMode::UNLOCK_WHILE_RUNNING,
#if defined(__linux__)
            0 // Default to disabled on linux, auto-resets briefly output black frames
#else
            5
#endif
        )
        , LOG_VIDEO_PATH_STATS(
            "<b>Log Video Path Diagnostics:</b><br>"
            "Once a second, log what the capture backend is actually delivering: pixel format, "
            "resolution, whether frames arrive on the CPU or as a GPU texture, and what it costs "
            "to copy one. Useful when diagnosing a capture card that renders wrong or burns CPU.",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
    {
        PA_ADD_OPTION(SHOW_ALL_FPS);
        PA_ADD_OPTION(VIDEO_BACKEND0);

        PA_ADD_OPTION(AUTO_RESET_SECONDS);
        PA_ADD_OPTION(LOG_VIDEO_PATH_STATS);
    }

public:
    BooleanCheckBoxOption SHOW_ALL_FPS;
    VideoBackendOption VIDEO_BACKEND0;

    SimpleIntegerOption<uint8_t> AUTO_RESET_SECONDS;
    BooleanCheckBoxOption LOG_VIDEO_PATH_STATS;
};



}
#endif
