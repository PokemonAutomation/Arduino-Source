/*  Tournament Prize Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV_ItemPrinterMaterialDetector.h"
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


ItemPrinterMaterialDetector::ItemPrinterMaterialDetector(Color color)
    : m_color(color)
    , m_box_mat_value(0.39, 0.623, 0.025, 0.050)
    , m_box_mat_quantity(0.485, 0.623, 0.037, 0.050)
{}

void ItemPrinterMaterialDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_mat_value);
    items.add(m_color, m_box_mat_quantity);
}
uint16_t ItemPrinterMaterialDetector::read_box(
    Logger& logger, AsyncDispatcher& dispatcher,
    const ImageViewRGB32& screen, const ImageFloatBox& box
) const{
    ImageViewRGB32 cropped = extract_box_reference(screen, box);

    std::vector<std::pair<ImageRGB32, size_t>> filtered = to_blackwhite_rgb32_range(
        cropped,
        {
            {0xff808080, 0xffffffff, true},
            {0xff909090, 0xffffffff, true},
            {0xffa0a0a0, 0xffffffff, true},
            {0xffb0b0b0, 0xffffffff, true},
            {0xffc0c0c0, 0xffffffff, true},
            {0xffd0d0d0, 0xffffffff, true},
            {0xffe0e0e0, 0xffffffff, true},
            {0xfff0f0f0, 0xffffffff, true},
        }
    );

    SpinLock lock;
    std::map<uint16_t, uint8_t> candidates;
    std::vector<std::unique_ptr<AsyncTask>> tasks(filtered.size());
    // for (size_t c = 0; c < filtered.size(); c++){
    //     filtered[c].first.save("DebugDumps/test-" + std::to_string(c) + ".png");
    // }
    for (size_t c = 0; c < filtered.size(); c++){
        tasks[c] = dispatcher.dispatch([&, c]{
            int num = OCR::read_number(logger, filtered[c].first);
            WriteSpinLock lg(lock);
            candidates[(uint16_t)num]++;
        });
    }

    //  Wait for everything.
    for (auto& task : tasks){
        task->wait_and_rethrow_exceptions();
    }

    std::pair<uint16_t, uint8_t> best;
    std::pair<uint16_t, uint8_t> second_best;
    for (const auto& item : candidates){
        logger.log("Candidate OCR: " + std::to_string(item.first) + "; x" + std::to_string(item.second));
        if (item.second >= best.second){
            second_best = best;
            best = item;
        }
        // std::cout << "Best: " << std::to_string(best.first) << "; x" << std::to_string(best.second) << std::endl;
        // std::cout << "Second Best: " << std::to_string(second_best.first) << "; x" << std::to_string(second_best.second) << std::endl;
    }

    if (best.second > second_best.second + 3){
        return best.first;
    }else{
        logger.log("There are disagreements among normal number OCR. Use Waterfill number OCR.");
        return (uint16_t)OCR::read_number_waterfill(logger, cropped, 0xff808080, 0xffffffff);
    }

}

// Press DPAD_RIGHT until a material with 68% value is on the screen,
// located at m_box_mat_value. This is likely to represent Happiny dust
void ItemPrinterMaterialDetector::find_happiny_dust(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context
) const{
    VideoSnapshot snapshot;
    for (size_t c = 0; c < 10; c++){
        context.wait_for_all_requests();
        snapshot = console.video().snapshot();
        uint16_t value = read_box(console, dispatcher, snapshot, m_box_mat_value);
        if (value == 68){
            return;
        }
        pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
    }

    throw OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT,
        console,
        "Failed to find Happiny dust after 10 tries.",
        std::move(snapshot.frame)
    );
}

uint16_t ItemPrinterMaterialDetector::detect_material_quantity(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context
) const{
    context.wait_for_all_requests();
    VideoSnapshot snapshot = console.video().snapshot();
    uint16_t value = read_box(console, dispatcher, snapshot, m_box_mat_quantity);
    return value;
}







}
}
}
