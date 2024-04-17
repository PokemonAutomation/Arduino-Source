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
    , m_box_normal(0.86, 0.34, 0.045, 0.050)
    , m_box_bonus(0.86, 0.423, 0.045, 0.050)
{}

void ItemPrinterJobsDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_normal);
    items.add(m_color, m_box_bonus);
}
uint8_t ItemPrinterJobsDetector::read_box(Logger& logger, const ImageViewRGB32& screen, const ImageFloatBox& box){
    ImageViewRGB32 cropped = extract_box_reference(screen, box);
    ImageRGB32 filtered = to_blackwhite_rgb32_range(cropped, 0xff808080, 0xffffffff, true);
    int num = OCR::read_number(logger, filtered);
    switch (num){
    case 1: return 1;
    case 5: return 5;
    case 10: return 10;
    }
    return 0;
}
uint8_t ItemPrinterJobsDetector::detect_jobs(Logger& logger, const ImageViewRGB32& screen) const{
    uint8_t normal = read_box(logger, screen, m_box_normal);
    if (normal != 0){
        return normal;
    }
    uint8_t bonus = read_box(logger, screen, m_box_bonus);
    return bonus;
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
