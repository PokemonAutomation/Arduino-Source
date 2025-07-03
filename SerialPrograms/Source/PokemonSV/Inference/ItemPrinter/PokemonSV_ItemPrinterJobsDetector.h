/*  Item Printer Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterJobsDetector_H
#define PokemonAutomation_PokemonSV_ItemPrinterJobsDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{

class ItemPrinterJobsDetector{
public:
    ItemPrinterJobsDetector(Color color);

    void make_overlays(VideoOverlaySet& items) const;
    uint8_t detect_jobs(Logger& logger, const ImageViewRGB32& screen) const;

    //  "jobs" jobs must be 1, 5, or 10.
    void set_print_jobs(
        VideoStream& stream, ProControllerContext& context, uint8_t jobs
    ) const;


private:
    std::pair<uint8_t, uint8_t> read_box(
        Logger& logger,
        const ImageViewRGB32& screen, const ImageFloatBox& box
    ) const;

private:
    Color m_color;
    ImageFloatBox m_box_normal;
    ImageFloatBox m_box_bonus;

};



}
}
}
#endif
