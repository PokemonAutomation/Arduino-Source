/*  Mount Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_MountDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;

class MountMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    using WaterfillTemplateMatcher::WaterfillTemplateMatcher;

#if 0
    virtual double rmsd(const QImage& image, const WaterfillObject& object) const override{
        if (!check_aspect_ratio(object.width(), object.height())){
//            cout << "bad aspect ratio" << endl;
            return 99999.;
        }
        if (!check_area_ratio(object.area_ratio())){
//            cout << "bad area ratio" << endl;
            return 99999.;
        }

//        static int c = 0;
//        cout << c << endl;

        QImage filtered = extract_box(image, object);
        filter_rgb32(object.packed_matrix(), filtered, COLOR_BLACK, true);

        double rmsd = WaterfillTemplateMatcher::rmsd(filtered);

        cout << "rmsd  = " << rmsd << endl;

//        if (rmsd <= m_max_rmsd){
            static int c = 0;
            filtered.save("test-" + QString::number(c++) + "-" + QString::number(rmsd) + ".png");
//        }

        return rmsd;
    }
#endif

};

class MountWyrdeerMatcher : public MountMatcher{
public:
    MountWyrdeerMatcher(bool on)
        : MountMatcher(
            on ? "PokemonLA/Mounts/MountOn-Wyrdeer-Template-1.png" : "PokemonLA/Mounts/MountOff-Wyrdeer-Template.png",
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
class MountUrsalunaMatcher : public MountMatcher{
public:
    MountUrsalunaMatcher(bool on)
        : MountMatcher(
            on ? "PokemonLA/Mounts/MountOn-Ursaluna-Template-1.png" : "PokemonLA/Mounts/MountOff-Ursaluna-Template.png",
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
class MountBasculegionMatcher : public MountMatcher{
public:
    MountBasculegionMatcher(bool on)
        : MountMatcher(
            on ? "PokemonLA/Mounts/MountOn-Basculegion-Template-1.png" : "PokemonLA/Mounts/MountOff-Basculegion-Template.png",
            Color(0xff808000), Color(0xffffffff), 100
        )
    {
        m_area_ratio_upper = 1.50;
    }
    static const MountBasculegionMatcher& on(){
        static MountBasculegionMatcher matcher(true);
        return matcher;
    }
    static const MountBasculegionMatcher& off(){
        static MountBasculegionMatcher matcher(false);
        return matcher;
    }
};
class MountSneaslerMatcher : public MountMatcher{
public:
    MountSneaslerMatcher(bool on)
        : MountMatcher(
            on ? "PokemonLA/Mounts/MountOn-Sneasler-Template-1.png" : "PokemonLA/Mounts/MountOff-Sneasler-Template.png",
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
class MountBraviaryMatcher : public MountMatcher{
public:
    MountBraviaryMatcher(bool on)
        : MountMatcher(
            on ? "PokemonLA/Mounts/MountOn-Braviary-Template-1.png" : "PokemonLA/Mounts/MountOff-Braviary-Template.png",
            Color(0xff808000), Color(0xffffffff), 100
        )
    {
        m_area_ratio_upper = 1.65;
    }
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
    double m_rmsd = 1000;
    MountState m_state = MountState::NOTHING;

    void add_filtered(double rmsd, MountState state){
        if (rmsd > 1500 || m_rmsd <= rmsd){
            return;
        }
        m_rmsd = rmsd;
        m_state = state;
    }
    void add_direct(double rmsd, MountState state){
        if (rmsd > 800 || m_rmsd <= rmsd){
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
        uint32_t filters[4][2]{
            {0xff707070, 0xffffffff},
            {0xff909090, 0xffffffff},
            {0xffb0b0b0, 0xffffffff},
            {0xffd0d0d0, 0xffffffff},
        };
        QImage filtered[4];
        filter4_rgb32_range(
            image,
            filtered[0], filters[0][0], filters[0][1], COLOR_BLACK, false,
            filtered[1], filters[1][0], filters[1][1], COLOR_BLACK, false,
            filtered[2], filters[2][0], filters[2][1], COLOR_BLACK, false,
            filtered[3], filters[3][0], filters[3][1], COLOR_BLACK, false
        );
        PackedBinaryMatrix matrix[4];
        compress4_rgb32_to_binary_range(
            image,
            matrix[0], filters[0][0], filters[0][1],
            matrix[1], filters[1][0], filters[1][1],
            matrix[2], filters[2][0], filters[2][1],
            matrix[3], filters[3][0], filters[3][1]
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
#if 1
                candidates.add_filtered(MountWyrdeerMatcher      ::off().rmsd(filtered[c], object), MountState::WYRDEER_OFF);
                candidates.add_direct  (MountWyrdeerMatcher      ::off().rmsd(image      , object), MountState::WYRDEER_OFF);
                candidates.add_filtered(MountUrsalunaMatcher     ::off().rmsd(filtered[c], object), MountState::URSALUNA_OFF);
                candidates.add_direct  (MountUrsalunaMatcher     ::off().rmsd(image      , object), MountState::URSALUNA_OFF);
                candidates.add_filtered(MountBasculegionMatcher  ::off().rmsd(filtered[c], object), MountState::BASCULEGION_OFF);
                candidates.add_direct  (MountBasculegionMatcher  ::off().rmsd(image      , object), MountState::BASCULEGION_OFF);
                candidates.add_filtered(MountSneaslerMatcher     ::off().rmsd(filtered[c], object), MountState::SNEASLER_OFF);
                candidates.add_direct  (MountSneaslerMatcher     ::off().rmsd(image      , object), MountState::SNEASLER_OFF);
                candidates.add_filtered(MountBraviaryMatcher     ::off().rmsd(filtered[c], object), MountState::BRAVIARY_OFF);
                candidates.add_direct  (MountBraviaryMatcher     ::off().rmsd(image      , object), MountState::BRAVIARY_OFF);
#endif
            }
        }
    }
    {
        int i = 0;
        uint32_t filters[4][2]{
            {0xff606000, 0xffffff7f},
            {0xff808000, 0xffffff6f},
            {0xffa0a000, 0xffffff5f},
            {0xffc0c000, 0xffffff4f},
        };
        QImage filtered[4];
        filter4_rgb32_range(
            image,
            filtered[0], filters[0][0], filters[0][1], COLOR_BLACK, false,
            filtered[1], filters[1][0], filters[1][1], COLOR_BLACK, false,
            filtered[2], filters[2][0], filters[2][1], COLOR_BLACK, false,
            filtered[3], filters[3][0], filters[3][1], COLOR_BLACK, false
        );
        PackedBinaryMatrix matrix[4];
        compress4_rgb32_to_binary_range(
            image,
            matrix[0], filters[0][0], filters[0][1],
            matrix[1], filters[1][0], filters[1][1],
            matrix[2], filters[2][0], filters[2][1],
            matrix[3], filters[3][0], filters[3][1]
        );
        for (size_t c = 0; c < 4; c++){
//            cout << "Filter: " << c << endl;
            WaterFillIterator finder(matrix[c], 50);
            WaterfillObject object;
            while (finder.find_next(object)){
                if (object.width() * 2 < (size_t)image.width() || object.width() == (size_t)image.width()){
                    continue;
                }
                if (object.height() * 3 < (size_t)image.height() || object.height() == (size_t)image.height()){
                    continue;
                }
#if 1
                candidates.add_filtered(MountWyrdeerMatcher      ::on().rmsd(filtered[c], object), MountState::WYRDEER_ON);
                candidates.add_direct  (MountWyrdeerMatcher      ::on().rmsd(image      , object), MountState::WYRDEER_ON);
                candidates.add_filtered(MountUrsalunaMatcher     ::on().rmsd(filtered[c], object), MountState::URSALUNA_ON);
                candidates.add_direct  (MountUrsalunaMatcher     ::on().rmsd(image      , object), MountState::URSALUNA_ON);
                candidates.add_filtered(MountBasculegionMatcher  ::on().rmsd(filtered[c], object), MountState::BASCULEGION_ON);
                candidates.add_direct  (MountBasculegionMatcher  ::on().rmsd(image      , object), MountState::BASCULEGION_ON);
                candidates.add_filtered(MountSneaslerMatcher     ::on().rmsd(filtered[c], object), MountState::SNEASLER_ON);
                candidates.add_direct  (MountSneaslerMatcher     ::on().rmsd(image      , object), MountState::SNEASLER_ON);
                candidates.add_filtered(MountBraviaryMatcher     ::on().rmsd(filtered[c], object), MountState::BRAVIARY_ON);
                candidates.add_direct  (MountBraviaryMatcher     ::on().rmsd(image      , object), MountState::BRAVIARY_ON);
#endif
                extract_box(image, object).save("test-" + QString::number(c) + "-" + QString::number(i) + ".png");
                i++;
            }
        }
//        cout << "i = " << i << endl;
    }

//    cout << "rmsd = " << candidates.m_rmsd << ", state = " << (int)candidates.m_state << endl;
    return candidates.m_state;
}





MountTracker::MountTracker(LoggerQt& logger)
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
//    cout << "state = " << (int)state << endl;

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
