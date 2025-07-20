/*  Let's Go Kill Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/Audio/SpectrogramMatcher.h"
#include "CommonTools/Audio/AudioTemplateCache.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV_LetsGoKillDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Kernels::Waterfill;


const ImageMatch::ExactImageMatcher& LETS_GO_KILL_CROPPED(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/LetsGoKill-Cropped.png");
    return matcher;
}


bool is_kill_icon(
    const ImageViewRGB32& image,
    WaterfillObject& object,
    const WaterfillObject& red, const WaterfillObject& white
){
//    cout << "is_kill_icon" << endl;
//    cout << red.min_x << "-" << red.max_x << ", " << white.min_y << "-" << white.max_y << endl;

#if 0
    static int c = 0;
    object = red;
    object.merge_assume_no_overlap(white);
    extract_box_reference(image, object).save("test-" + std::to_string(c++) + ".png");
#endif

    if (red.max_y >= white.min_y){
//        cout << "bad y" << endl;
        return false;
    }
    if (red.min_x < white.min_x){
//        cout << "bad min_x" << endl;
        return false;
    }
    if (red.max_x < white.max_x){
//        cout << "bad max_x" << endl;
        return false;
    }

    object = red;
    object.merge_assume_no_overlap(white);

    if (object.width() < 18 || object.height() < 18){
//        cout << "too small: " << object.width() << endl;
        return false;
    }
    double aspect_ratio = object.aspect_ratio();
    if (aspect_ratio < 0.8 || aspect_ratio > 1.2){
//        cout << aspect_ratio << endl;
        return false;
    }

    ImageViewRGB32 cropped = extract_box_reference(image, object);
//    cropped.save("test.png");
    double rmsd = LETS_GO_KILL_CROPPED().rmsd(cropped);
//    cout << rmsd << endl;
    return rmsd < 120;
}




LetsGoKillDetector::LetsGoKillDetector(
    Color color,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_box(box)
{}
void LetsGoKillDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool LetsGoKillDetector::detect(const ImageViewRGB32& screen){
    using namespace Kernels::Waterfill;

    size_t size_threshold = (size_t)(10. * screen.total_pixels() / 2073600);

    ImageViewRGB32 region = extract_box_reference(screen, m_box);
    std::vector<WaterfillObject> reds;
    std::vector<WaterfillObject> whites;
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    {
        std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
            region,
            {
                {0xff801e1e, 0xffff5f5f},
                {0xff901e1e, 0xffff5f5f},
                {0xffa01e1e, 0xffff5f5f},
                {0xffb01e1e, 0xffff5f5f},
                {0xffc01e1e, 0xffff5f5f},
                {0xffd01e1e, 0xffff5f5f},
                {0xff801e1e, 0xffff7f7f},
                {0xff901e1e, 0xffff7f7f},
                {0xffa01e1e, 0xffff7f7f},
                {0xffb01e1e, 0xffff7f7f},
                {0xffc01e1e, 0xffff7f7f},
                {0xffd01e1e, 0xffff7f7f},
            }
        );
//        size_t c = 0;
        for (PackedBinaryMatrix& matrix : matrices){
            session->set_source(matrix);
            auto iter = session->make_iterator(size_threshold);
            WaterfillObject object;
            while (iter->find_next(object, false)){
                double aspect_ratio = object.aspect_ratio();
                if (aspect_ratio < 1.5 || aspect_ratio > 2.5){
                    continue;
                }
//                cout << object.area << endl;
//                extract_box_reference(region, object).save("red-" + std::to_string(c++) + ".png");
                reds.emplace_back(std::move(object));
            }
        }
    }
    {
        std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
            region,
            {
                {0xffb0b0b0, 0xffffffff},
                {0xffc0c0c0, 0xffffffff},
                {0xffd0d0d0, 0xffffffff},
                {0xffe0e0e0, 0xffffffff},
            }
        );
//        size_t c = 0;
        for (PackedBinaryMatrix& matrix : matrices){
            session->set_source(matrix);
            auto iter = session->make_iterator(size_threshold);
            WaterfillObject object;
            while (iter->find_next(object, false)){
                double aspect_ratio = object.aspect_ratio();
                if (aspect_ratio < 1.5 || aspect_ratio > 2.7){
                    continue;
                }
//                cout << object.area << endl;
//                extract_box_reference(region, object).save("white-" + std::to_string(c++) + ".png");
                whites.emplace_back(std::move(object));
            }
        }
    }
    for (WaterfillObject& red : reds){
        for (WaterfillObject& white : whites){
            WaterfillObject object;
            if (is_kill_icon(region, object, red, white)){
                return true;
            }
        }
    }

    return false;
}



LetsGoKillWatcher::LetsGoKillWatcher(
    Logger& logger,
    Color color, bool trigger_if_detected,
    const ImageFloatBox& box,
    std::function<void()> on_kill_callback,
    std::chrono::milliseconds duration
)
     : DetectorToFinder("LetsGoKillWatcher", duration, color, box)
     , m_logger(logger)
     , m_trigger_if_detected(trigger_if_detected)
     , m_on_kill_callback(on_kill_callback)
     , m_last_detection(false)
     , m_last_detected(WallClock::min())
{}
bool LetsGoKillWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool result = DetectorToFinder::process_frame(frame, timestamp);
    if (!result){
        m_last_detection = false;
        return false;
    }

    if (m_last_detected.load(std::memory_order_relaxed) == WallClock::min()){
        m_logger.log("Detected Let's Go kill icon.");
    }
    m_last_detected.store(timestamp, std::memory_order_release);

    if (!m_last_detection && m_on_kill_callback){
        m_on_kill_callback();
    }
    m_last_detection = true;
    return m_trigger_if_detected;
}




LetsGoKillSoundDetector::LetsGoKillSoundDetector(Logger& logger, DetectedCallback detected_callback)
    : AudioPerSpectrumDetectorBase(
        logger,
        "LetsGoKillSoundDetector",
        "Let's Go Kill Sound",
        COLOR_RED,
        [this, callback = std::move(detected_callback)](float error_coefficient){
            m_last_detected = current_time();
            return callback != nullptr ? callback(error_coefficient) : false;
        }
    )
    , m_last_detected(WallClock::min())
{}
float LetsGoKillSoundDetector::get_score_threshold() const{
    return (float)GameSettings::instance().LETS_GO_KILL_SOUND_THRESHOLD;
}
std::unique_ptr<SpectrogramMatcher> LetsGoKillSoundDetector::build_spectrogram_matcher(size_t sample_rate){
    return std::make_unique<SpectrogramMatcher>(
        "Let's Go Kill",
        AudioTemplateCache::instance().get_throw("PokemonSV/LetsGoKill", sample_rate),
        SpectrogramMatcher::Mode::SPIKE_CONV, sample_rate,
        GameSettings::instance().LETS_GO_KILL_SOUND_LOW_FREQUENCY
    );
}








}
}
}
