/*  Item Printer Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterJobsDetector_H
#define PokemonAutomation_PokemonSV_ItemPrinterJobsDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class Logger;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{

class ItemPrinterJobsDetector{
public:
    ItemPrinterJobsDetector(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    uint8_t detect_jobs(Logger& logger, const ImageViewRGB32& screen) const;

    //  "jobs" jobs must be 1, 5, or 10.
    void set_print_jobs(
        ConsoleHandle& console, BotBaseContext& context, uint8_t jobs
    ) const;


private:
    Color m_color;
    ImageFloatBox m_box;

};



}
}
}
#endif
