/*  Date Manipulation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateManipBase_H
#define PokemonAutomation_NintendoSwitch_DateManipBase_H

#include <memory>
#include "Common/Cpp/DateTime.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class Logger;
    class ImageRGB32;
    class VideoOverlaySet;
    class VideoStream;
namespace NintendoSwitch{


class DateReaderBase{
public:
    virtual void make_overlays(VideoOverlaySet& items) const = 0;
    virtual DateTime read_date(
        Logger& logger,
        std::shared_ptr<const ImageRGB32> screen
    ) = 0;
    virtual void set_date(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        const DateTime& date    //  Seconds is ignored.
    ) = 0;
};




}
}
#endif
