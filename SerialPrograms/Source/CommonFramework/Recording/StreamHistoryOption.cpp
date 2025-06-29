/*  Stream History Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "StreamHistoryOption.h"

namespace PokemonAutomation{

StreamHistoryOption::StreamHistoryOption()
    : GroupOption(
        "Stream History",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_DISABLED,
        true
    )
    , DESCRIPTION(
        "Keep a record of the recent video+audio streams. This will allow video capture "
        "for unexpected events.<br><br>"
        "<font color=\"red\">Warning: This feature has a known memory leak. It will leak ~3GB per day per "
        "video stream. You have been warned!</font>"
        "<br><br>"
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
    , RESOLUTION(
        "<b>Resolution:</b>",
        {
            {Resolution::MATCH_INPUT,   "match",    "Match Input Resolution"},
            {Resolution::FORCE_720p,    "720p",     "1280 x 720"},
            {Resolution::FORCE_1080p,   "1080p",    "1920 x 1080"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        Resolution::FORCE_720p
    )
    , ENCODING_MODE(
        "<b>Encoding Mode:</b>",
        {
            {EncodingMode::FIXED_QUALITY, "fixed-quality", "Fixed Quality"},
            {EncodingMode::FIXED_BITRATE, "fixed-bitrate", "Fixed Bit Rate"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        EncodingMode::FIXED_QUALITY
    )
    , VIDEO_QUALITY(
        "<b>Video Quality:</b><br>"
        "<font color=\"orange\">High quality videos will take more disk space "
        "and may exceed the attachment size limit for Discord notifications."
        "</font>",
        {
            {VideoQuality::VERY_LOW,    "very-low",     "Very Low"},
            {VideoQuality::LOW,         "low",          "Low"},
            {VideoQuality::NORMAL,      "normal",       "Normal"},
            {VideoQuality::HIGH,        "high",         "High"},
            {VideoQuality::VERY_HIGH,   "very-high",    "Very High"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        VideoQuality::LOW
    )
    , VIDEO_BITRATE(
        "<b>Video Bit-Rate (kbps):</b><br>"
        "Lower = lower quality, smaller file size.<br>"
        "Higher = high quality, larger file size.<br>"
        "<font color=\"orange\">Large values can exceed the attachment size limit for Discord notifications."
        "</font>",
        LockMode::UNLOCK_WHILE_RUNNING,
        5000
    )
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(HISTORY_SECONDS);
    PA_ADD_OPTION(RESOLUTION);
    PA_ADD_OPTION(ENCODING_MODE);
    PA_ADD_OPTION(VIDEO_QUALITY);
    PA_ADD_OPTION(VIDEO_BITRATE);

    StreamHistoryOption::on_config_value_changed(this);

    ENCODING_MODE.add_listener(*this);
}
StreamHistoryOption::~StreamHistoryOption(){
    ENCODING_MODE.remove_listener(*this);
}

void StreamHistoryOption::on_config_value_changed(void* object){
    switch (ENCODING_MODE){
    case EncodingMode::FIXED_QUALITY:
        VIDEO_QUALITY.set_visibility(ConfigOptionState::ENABLED);
        VIDEO_BITRATE.set_visibility(ConfigOptionState::HIDDEN);
        break;
    case EncodingMode::FIXED_BITRATE:
        VIDEO_QUALITY.set_visibility(ConfigOptionState::HIDDEN);
        VIDEO_BITRATE.set_visibility(ConfigOptionState::ENABLED);
        break;
    }
}






}
