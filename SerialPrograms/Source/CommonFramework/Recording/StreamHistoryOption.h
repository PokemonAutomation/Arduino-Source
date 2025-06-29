/*  Stream History Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamHistoryOption_H
#define PokemonAutomation_StreamHistoryOption_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/GroupOption.h"

namespace PokemonAutomation{



class StreamHistoryOption : public GroupOption, public ConfigOption::Listener{
public:
    StreamHistoryOption();
    ~StreamHistoryOption();

    virtual void on_config_value_changed(void* object) override;

    StaticTextOption DESCRIPTION;
    SimpleIntegerOption<uint16_t> HISTORY_SECONDS;

    enum class Resolution{
        MATCH_INPUT,
        FORCE_720p,
        FORCE_1080p,
    };
    EnumDropdownOption<Resolution> RESOLUTION;

    enum class EncodingMode{
        FIXED_QUALITY,
        FIXED_BITRATE,
    };
    EnumDropdownOption<EncodingMode> ENCODING_MODE;


    enum class VideoQuality{
        VERY_LOW,
        LOW,
        NORMAL,
        HIGH,
        VERY_HIGH,
    };
    EnumDropdownOption<VideoQuality> VIDEO_QUALITY;
    SimpleIntegerOption<uint32_t> VIDEO_BITRATE;
};


}
#endif
