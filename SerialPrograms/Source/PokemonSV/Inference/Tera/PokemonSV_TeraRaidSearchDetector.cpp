/*  Tera Raid Search Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_TeraRaidSearchDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class TeraSearchGlassMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    TeraSearchGlassMatcher()
        : WaterfillTemplateMatcher("PokemonSV/TeraSearchGlass.png", Color(0xff000000), Color(0xff808080), 100)
    {}

    static const TeraSearchGlassMatcher& instance(){
        static TeraSearchGlassMatcher self;
        return self;
    }
};



TeraRaidSearchDetector::TeraRaidSearchDetector(Color color)
    : m_color(color)
//    , m_arrow_offline(color, GradientArrowType::DOWN, {0.475, 0.277, 0.050, 0.080})
{}
void TeraRaidSearchDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, {0, 0, 1, 1});
//    m_arrow_offline.make_overlays(items);
}
bool TeraRaidSearchDetector::detect(const ImageViewRGB32& screen){
    ImageFloatBox box;
    return detect_search_location(box, screen);
}
bool TeraRaidSearchDetector::detect_search_location(ImageFloatBox& box, const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(screen, 0xff000000, 0xff808080);
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;

//    static size_t c = 0;
    const TeraSearchGlassMatcher& MATCHER = TeraSearchGlassMatcher::instance();
    while (iter->find_next(object, false)){
//        cout << "yellow = " << object.area << endl;
//        extract_box_reference(screen, object).save("object-" + std::to_string(c++) + ".png");
//        yellows.emplace_back(std::move(object));
        double rmsd = MATCHER.rmsd(extract_box_reference(screen, object));
//        cout << "rmsd = " << rmsd << endl;
        if (rmsd < 100){

//            cout << "rmsd = " << rmsd << endl;
//            extract_box_reference(screen, object).save("object-" + std::to_string(c++) + ".png");

            box = translate_to_parent(screen, {0, 0, 1, 1}, object);
            return true;
        }
    }

    return false;
}
bool TeraRaidSearchDetector::move_cursor_to_search(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
) const{
    GradientArrowDetector arrow(m_color, GradientArrowType::DOWN, {0, 0, 1, 1});

    size_t consecutive_detection_fails = 0;
    size_t moves = 0;
    bool target_reached = false;
    while (true){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();

        ImageFloatBox search_location;
        ImageFloatBox arrow_location;

//        cout << "asdf" << endl;

        bool ok = true;
        ok &= detect_search_location(search_location, screen);
        ok &= arrow.detect(arrow_location, screen);
        if (!ok){
            consecutive_detection_fails++;
            if (consecutive_detection_fails > 3){
//                dump_image_and_throw_recoverable_exception(info, stream, "UnableToDetectTeraSearch", "Unable to detect Tera Raid Search menu.");
                stream.log("Unable to detect Tera Raid Search menu.", COLOR_RED);
                return false;
            }
            context.wait_for(std::chrono::milliseconds(1000));
            continue;
        }


        if (moves >= 10){
            stream.log("Unable to move to target after 10 moves.", COLOR_RED);
            return false;
        }
        moves++;

        double target_x = search_location.x + search_location.width * 0.5;
        double target_y = search_location.y - search_location.height * 0.5;
        double arrow_x = arrow_location.x + arrow_location.width * 0.5;
        double arrow_y = arrow_location.y + arrow_location.height * 0.5;

        double diff_x = target_x - arrow_x;
//        cout << "diff_x = " << diff_x << endl;
        if (diff_x < -0.1 || diff_x > 0.5){
            target_reached = false;
            pbf_press_dpad(context, DPAD_LEFT, 20, 10);
            continue;
        }
        if (diff_x > 0.1 || diff_x < -0.5){
            target_reached = false;
            pbf_press_dpad(context, DPAD_RIGHT, 20, 10);
            continue;
        }

        double diff_y = target_y - arrow_y;
//        cout << "diff_y = " << diff_y << endl;
        if (std::abs(diff_y) > 0.3){
            target_reached = false;
            pbf_press_dpad(context, DPAD_DOWN, 20, 10);
            continue;
        }

        //  Don't return yet. Wait a second to make sure the video is
        //  in steady state before we return.
        if (target_reached){
            return true;
        }
        target_reached = true;
        context.wait_for(std::chrono::seconds(1));
    }
}



CodeEntryDetector::CodeEntryDetector(Color color)
    : m_color(color)
    , m_bottom(0.15, 0.92, 0.30, 0.07)
    , m_left(0.01, 0.55, 0.02, 0.30)
    , m_right(0.97, 0.55, 0.02, 0.30)
    , m_center(0.20, 0.47, 0.60, 0.50)
{}

void CodeEntryDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    items.add(m_color, m_left);
    items.add(m_color, m_right);
    items.add(m_color, m_center);
}
bool CodeEntryDetector::detect(const ImageViewRGB32& screen){
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//    cout << bottom.average << bottom.stddev << endl;

    bool light_theme = bottom.average.sum() > 500;

    if (light_theme){
        if (!is_white(bottom)){
//            cout << "bottom" << endl;
            return false;
        }
        ImageStats left = image_stats(extract_box_reference(screen, m_left));
//        cout << left.average << left.stddev << endl;
        if (!is_white(left)){
//            cout << "left" << endl;
            return false;
        }
        ImageStats right = image_stats(extract_box_reference(screen, m_right));
        if (!is_white(right)){
//            cout << "right" << endl;
            return false;
        }
    }else{
        if (!is_grey(bottom, 50, 300)){
            return false;
        }
        ImageStats left = image_stats(extract_box_reference(screen, m_left));
        if (!is_grey(left, 50, 300)){
            return false;
        }
        ImageStats right = image_stats(extract_box_reference(screen, m_right));
        if (!is_grey(right, 50, 300)){
            return false;
        }
    }

    ImageStats center = image_stats(extract_box_reference(screen, m_center));
//    cout << center.average << center.stddev << endl;
    if (center.stddev.sum() < 50){
        return false;
    }

    return true;
}






}
}
}
