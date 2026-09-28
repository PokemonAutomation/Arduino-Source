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
        , FORCE_FULL_COLOR_RANGE(
            "<b>Force Full Color Range:</b><br>"
            "Treat YUV frames as full range (0-255) instead of whatever the capture device "
            "reports. Many capture cards deliver full range YUV but do not say so, which makes "
            "saturated colors come out over-saturated and breaks color-based detection. "
            "Symptom: detectors never recognize a state even though the video looks fine. "
            "Only affects the frames sent to detection, not the on-screen preview.",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
        , USE_QT_YUV_CONVERSION(
            "<b>Use Qt's YUV Conversion:</b><br>"
            "Fall back to Qt for converting captured frames to RGB. Qt ignores the color "
            "range the frame declares and always assumes limited range, which over-saturates "
            "full range sources and breaks color-based detection. It is also much slower. "
            "Only turn this on if our conversion misbehaves on your hardware.",
            LockMode::UNLOCK_WHILE_RUNNING,
            false
        )
    {
        PA_ADD_OPTION(SHOW_ALL_FPS);
        PA_ADD_OPTION(VIDEO_BACKEND0);

        PA_ADD_OPTION(AUTO_RESET_SECONDS);
        PA_ADD_OPTION(FORCE_FULL_COLOR_RANGE);
        PA_ADD_OPTION(USE_QT_YUV_CONVERSION);
    }

public:
    BooleanCheckBoxOption SHOW_ALL_FPS;
    VideoBackendOption VIDEO_BACKEND0;

    SimpleIntegerOption<uint8_t> AUTO_RESET_SECONDS;
    BooleanCheckBoxOption FORCE_FULL_COLOR_RANGE;
    BooleanCheckBoxOption USE_QT_YUV_CONVERSION;
};



}
#endif
