/*  Mount Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_MountDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;


class MountWyrdeerMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MountWyrdeerMatcher(bool on)
        : WaterfillTemplateMatcher(
            on ? "PokemonLA/Mounts/MountOn-Wyrdeer-Template.png" : "PokemonLA/Mounts/MountOff-Wyrdeer-Template.png",
            Color(0xff808000), Color(0xffffffff), 100
        )
    {}
    static const MountWyrdeerMatcher& on(){
        static MountWyrdeerMatcher matcher(true);
        return matcher;
    }
    static const MountWyrdeerMatcher& off(){
        static MountWyrdeerMatcher matcher(false);
        return matcher;
    }
};
class MountUrsalunaMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MountUrsalunaMatcher(bool on)
        : WaterfillTemplateMatcher(
            on ? "PokemonLA/Mounts/MountOn-Ursaluna-Template.png" : "PokemonLA/Mounts/MountOff-Ursaluna-Template.png",
            Color(0xff808000), Color(0xffffffff), 100
        )
    {}
    static const MountUrsalunaMatcher& on(){
        static MountUrsalunaMatcher matcher(true);
        return matcher;
    }
    static const MountUrsalunaMatcher& off(){
        static MountUrsalunaMatcher matcher(false);
        return matcher;
    }
};
class MountBasculegionMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MountBasculegionMatcher(bool on)
        : WaterfillTemplateMatcher(
            on ? "PokemonLA/Mounts/MountOn-Basculegion-Template.png" : "PokemonLA/Mounts/MountOff-Basculegion-Template.png",
            Color(0xff808000), Color(0xffffffff), 100
        )
    {}
    static const MountBasculegionMatcher& on(){
        static MountBasculegionMatcher matcher(true);
        return matcher;
    }
    static const MountBasculegionMatcher& off(){
        static MountBasculegionMatcher matcher(false);
        return matcher;
    }
};
class MountSneaslerMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MountSneaslerMatcher(bool on)
        : WaterfillTemplateMatcher(
            on ? "PokemonLA/Mounts/MountOn-Sneasler-Template.png" : "PokemonLA/Mounts/MountOff-Sneasler-Template.png",
            Color(0xff808000), Color(0xffffffff), 100
        )
    {}
    static const MountSneaslerMatcher& on(){
        static MountSneaslerMatcher matcher(true);
        return matcher;
    }
    static const MountSneaslerMatcher& off(){
        static MountSneaslerMatcher matcher(false);
        return matcher;
    }
};
class MountBraviaryMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MountBraviaryMatcher(bool on)
        : WaterfillTemplateMatcher(
            on ? "PokemonLA/Mounts/MountOn-Braviary-Template.png" : "PokemonLA/Mounts/MountOff-Braviary-Template.png",
            Color(0xff808000), Color(0xffffffff), 100
        )
    {}
    static const MountBraviaryMatcher& on(){
        static MountBraviaryMatcher matcher(true);
        return matcher;
    }
    static const MountBraviaryMatcher& off(){
        static MountBraviaryMatcher matcher(false);
        return matcher;
    }
};




const char* MOUNT_STATE_STRINGS[] = {
    "No Detection",
    "Wrydeer Off",
    "Wrydeer On",
    "Ursaluna Off",
    "Ursaluna On",
    "Basculegion Off",
    "Basculegion On",
    "Sneasler Off",
    "Sneasler On",
    "Braviary Off",
    "Braviary On",
};



MountDetector::MountDetector()
    : m_box(0.905, 0.65, 0.08, 0.13)
{}

struct MountCandiateTracker{
    double m_rmsd = 80;
    MountState m_state = MountState::NOTHING;

    void add(double rmsd, MountState state){
        if (m_rmsd <= rmsd){
            return;
        }
        m_rmsd = rmsd;
        m_state = state;
    }
};

void MountDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
MountState MountDetector::detect(const QImage& screen) const{
    QImage image = extract_box(screen, m_box);

    MountCandiateTracker candidates;

    {
        PackedBinaryMatrix matrix[4];
        compress4_rgb32_to_binary_range(
            image,
            matrix[0], 0xff707070, 0xffffffff,
            matrix[1], 0xff909090, 0xffffffff,
            matrix[2], 0xffb0b0b0, 0xffffffff,
            matrix[3], 0xffd0d0d0, 0xffffffff
        );
        for (size_t c = 0; c < 4; c++){
            WaterFillIterator finder(matrix[c], 50);
            WaterfillObject object;
            while (finder.find_next(object)){
                if (object.width() * 2 < (size_t)image.width() || object.width() == (size_t)image.width()){
                    continue;
                }
                if (object.height() * 3 < (size_t)image.height() || object.height() == (size_t)image.height()){
                    continue;
                }
                candidates.add(MountWyrdeerMatcher::off().rmsd(image, object), MountState::WYRDEER_OFF);
                candidates.add(MountUrsalunaMatcher::off().rmsd(image, object), MountState::URSALUNA_OFF);
                candidates.add(MountBasculegionMatcher::off().rmsd(image, object), MountState::BASCULEGION_OFF);
                candidates.add(MountSneaslerMatcher::off().rmsd(image, object), MountState::SNEASLER_OFF);
                candidates.add(MountBraviaryMatcher::off().rmsd(image, object), MountState::BRAVIARY_OFF);
            }
        }
    }
    {
        PackedBinaryMatrix matrix[4];
        compress4_rgb32_to_binary_range(
            image,
            matrix[0], 0xff606000, 0xffffff7f,
            matrix[1], 0xff808000, 0xffffff6f,
            matrix[2], 0xffa0a000, 0xffffff5f,
            matrix[3], 0xffc0c000, 0xffffff4f
        );
        for (size_t c = 0; c < 4; c++){
            WaterFillIterator finder(matrix[c], 50);
            WaterfillObject object;
            while (finder.find_next(object)){
                if (object.width() * 2 < (size_t)image.width() || object.width() == (size_t)image.width()){
                    continue;
                }
                if (object.height() * 3 < (size_t)image.height() || object.height() == (size_t)image.height()){
                    continue;
                }
                candidates.add(MountWyrdeerMatcher::on().rmsd(image, object), MountState::WYRDEER_ON);
                candidates.add(MountUrsalunaMatcher::on().rmsd(image, object), MountState::URSALUNA_ON);
                candidates.add(MountBasculegionMatcher::on().rmsd(image, object), MountState::BASCULEGION_ON);
                candidates.add(MountSneaslerMatcher::on().rmsd(image, object), MountState::SNEASLER_ON);
                candidates.add(MountBraviaryMatcher::on().rmsd(image, object), MountState::BRAVIARY_ON);
            }
        }
    }

//    cout << "rmsd = " << candidates.m_rmsd << endl;
    return candidates.m_state;
}





MountTracker::MountTracker(Logger& logger)
    : VisualInferenceCallback("MountTracker")
    , m_logger(logger)
    , m_state(MountState::NOTHING)
{}

void MountTracker::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

MountState MountTracker::state() const{
    return m_state.load(std::memory_order_acquire);
}

bool MountTracker::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    MountState state = m_detector.detect(frame);

//    SpinLockGuard lg(m_lock);

    //  Clear out old history.
    std::chrono::system_clock::time_point threshold = timestamp - std::chrono::seconds(1);
    while (!m_history.empty()){
        Sample& sample = m_history.front();
        if (m_history.front().timestamp >= threshold){
            break;
        }
        m_counts[sample.state]--;
        m_history.pop_front();
    }

    size_t& count = m_counts[state];
    m_history.emplace_back(Sample{timestamp, state});
    count++;

    //  Return most reported state in the last window.
    MountState best_state = MountState::NOTHING;
    size_t best_count = 0;
    for (const auto& item : m_counts){
        if (best_count < item.second){
            best_count = item.second;
            best_state = item.first;
        }
    }

    MountState last_state = this->state();
    if (last_state != best_state){
        m_logger.log(
            std::string("Mount changed from ") + MOUNT_STATE_STRINGS[(int)last_state] +
            " to " + MOUNT_STATE_STRINGS[(int)best_state] + ".",
            COLOR_PURPLE
        );
    }
    m_state.store(best_state, std::memory_order_release);

    return false;
}








}
}
}
