/*  Item Printer Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ItemPrinterJobsDetector_H
#define PokemonAutomation_PokemonSV_ItemPrinterJobsDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class Logger;
    class AsyncDispatcher;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{

class ItemPrinterJobsDetector{
public:
    ItemPrinterJobsDetector(Color color, Language language);

    void make_overlays(VideoOverlaySet& items) const;
    uint8_t detect_jobs(Logger& logger, AsyncDispatcher& dispatcher, const ImageViewRGB32& screen) const;

    //  "jobs" jobs must be 1, 5, or 10.
    void set_print_jobs(
        AsyncDispatcher& dispatcher,
        ConsoleHandle& console, BotBaseContext& context, uint8_t jobs
    ) const;


private:
    std::pair<uint8_t, uint8_t> read_box(
        Logger& logger, AsyncDispatcher& dispatcher,
        const ImageViewRGB32& screen, const ImageFloatBox& box
    ) const;

private:
    Color m_color;
    Language m_language;
    ImageFloatBox m_box_normal;
    ImageFloatBox m_box_bonus;

};



}
}
}
#endif
