/*  Tournament Prize Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV_ItemPrinterJobsDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


ItemPrinterJobsDetector::ItemPrinterJobsDetector(Color color)
    : m_color(color)
    , m_box(0.86, 0.34, 0.045, 0.050)
{}

void ItemPrinterJobsDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
uint8_t ItemPrinterJobsDetector::detect_jobs(Logger& logger, const ImageViewRGB32& screen) const{
    ImageViewRGB32 box = extract_box_reference(screen, m_box);
    ImageRGB32 filtered = to_blackwhite_rgb32_range(box, 0xff808080, 0xffffffff, true);
    int num = OCR::read_number(logger, filtered);
    switch (num){
    case 1: return 1;
    case 5: return 5;
    case 10: return 10;
    }
    return 0;
}


void ItemPrinterJobsDetector::set_print_jobs(
    ConsoleHandle& console, BotBaseContext& context, uint8_t jobs
) const{
    VideoSnapshot snapshot;
    for (size_t c = 0; c < 10; c++){
        context.wait_for_all_requests();
        snapshot = console.video().snapshot();
        uint8_t current_jobs = detect_jobs(console, snapshot);
        if (current_jobs == jobs){
            return;
        }
        pbf_press_button(context, BUTTON_R, 20, 30);
    }

    throw OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT,
        console,
        "Failed to set jobs after 10 tries.",
        std::move(snapshot.frame)
    );
}







}
}
}
