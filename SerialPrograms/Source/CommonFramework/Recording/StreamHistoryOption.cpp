/*  Stream History Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "StreamHistoryOption.h"

namespace PokemonAutomation{


StreamHistoryOption::StreamHistoryOption()
    : GroupOption(
        "Stream History",
        LockMode::LOCK_WHILE_RUNNING,
        true,
        IS_BETA_VERSION
    )
    , DESCRIPTION(
        "Keep a record of the recent video+audio streams. This will allow video capture "
        "for unexpected events.<br><br>"
        "<font color=\"orange\">Warning: This feature is computationally expensive and "
        "will require a more powerful computer to run (especially for multi-Switch programs).<br>"
        "Furthermore, the current implementation is inefficient as it will write a lot "
        "of data to disk. This feature is still a work-in-progress."
        "</font>"
    )
    , HISTORY_SECONDS(
        "<b>History (seconds):</b><br>"
        "Keep this many seconds of video and audio feed for video capture and debugging purposes.<br><br>"
        "<font color=\"orange\">Do not set this too large as it will consume a lot of memory and may exceed the "
        "attachment size limit for Discord notifications."
        "</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        30
    )
    , VIDEO_BITRATE(
        "<b>Video Bit-Rate (kbps):</b><br>"
        "Lower = lower quality, smaller file size.<br>"
        "Higher = high quality, larger file size.<br><br>"
        "<font color=\"orange\">Large values can exceed the attachment size limit for Discord notifications."
        "</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        5000
    )
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(HISTORY_SECONDS);
    PA_ADD_OPTION(VIDEO_BITRATE);
}



}
