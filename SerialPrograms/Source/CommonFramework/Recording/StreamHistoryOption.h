/*  Stream History Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_StreamHistoryOption_H
#define PokemonAutomation_StreamHistoryOption_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/GroupOption.h"

namespace PokemonAutomation{



class StreamHistoryOption : public GroupOption{
public:
    StreamHistoryOption();

    StaticTextOption DESCRIPTION;
    SimpleIntegerOption<uint16_t> HISTORY_SECONDS;
    SimpleIntegerOption<uint32_t> VIDEO_BITRATE;
};


}
#endif
