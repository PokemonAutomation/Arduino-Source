/*  Tournament Prize Jobs Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
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
std::pair<uint8_t, uint8_t> ItemPrinterJobsDetector::read_box(
    Logger& logger,
    const ImageViewRGB32& screen, const ImageFloatBox& box
) const{
    ImageViewRGB32 cropped = extract_box_reference(screen, box);

    std::vector<std::pair<ImageRGB32, size_t>> filtered = to_blackwhite_rgb32_range(
        cropped,
        {
            {true, 0xff808080, 0xffffffff},
            {true, 0xff909090, 0xffffffff},
            {true, 0xffa0a0a0, 0xffffffff},
            {true, 0xffb0b0b0, 0xffffffff},
            {true, 0xffc0c0c0, 0xffffffff},
            {true, 0xffd0d0d0, 0xffffffff},
            {true, 0xffe0e0e0, 0xffffffff},
            {true, 0xfff0f0f0, 0xffffffff},
        }
    );

    SpinLock lock;
    std::map<uint8_t, uint8_t> candidates;

    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            int num = OCR::read_number(logger, filtered[index].first);
            std::string str = std::to_string(num);
            WriteSpinLock lg(lock);
            if (str == "1"){
                candidates[1]++;
            }else if (str == "5"){
                candidates[5]++;
            }else if (str == "10"){
                candidates[10]++;
            }else if (str[0] == '5'){
                candidates[5]++;
            }else if (str[0] == '1' && str[1] == '0'){
                candidates[10]++;
            }else if (str[0] == '1' && str[1] == '5'){
            }else if (str[0] == '1'){
                candidates[1]++;
            }
        },
        0, filtered.size(), 1
    );

    std::pair<uint8_t, uint8_t> best;
    for (const auto& item : candidates){
        if (item.second > best.second){
            best = item;
        }
    }

    return best;


#if 0
    ImageRGB32 filtered = to_blackwhite_rgb32_range(cropped, 0xffc0c0c0, 0xffffffff, true);

    static int c = 0;
    filtered.save("test-" + std::to_string(c++) + ".png");

    int num = OCR::read_number(logger, filtered, m_language);
    std::string str = std::to_string(num);
    if (str[0] == '5'){
        return 5;
    }
    if (str[0] == '1' && str[1] == '0'){
        return 10;
    }
    if (str[0] == '1'){
        return 1;
    }
    return 0;
#endif
}
uint8_t ItemPrinterJobsDetector::detect_jobs(Logger& logger, const ImageViewRGB32& screen) const{
    std::pair<uint8_t, uint8_t> normal = read_box(logger, screen, m_box_normal);
    if (normal.second > 6){
        return normal.first;
    }
    std::pair<uint8_t, uint8_t> bonus = read_box(logger, screen, m_box_bonus);
    if (normal.second + 2 > bonus.second){
        return normal.first;
    }else{
        return bonus.first;
    }
}


void ItemPrinterJobsDetector::set_print_jobs(
    VideoStream& stream, ProControllerContext& context, uint8_t jobs
) const{
    VideoSnapshot snapshot;
    for (size_t c = 0; c < 10; c++){
        context.wait_for_all_requests();
        snapshot = stream.video().snapshot();
        uint8_t current_jobs = detect_jobs(stream.logger(), snapshot);
        if (current_jobs == jobs){
            return;
        }
        pbf_press_button(context, BUTTON_R, 20, 30);
    }

    throw_and_log<OperationFailedException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "Failed to set jobs after 10 tries.",
        &stream,
        std::move(snapshot.frame)
    );
}







}
}
}
