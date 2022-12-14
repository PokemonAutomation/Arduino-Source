/*  Egg Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"
#include "PokemonSV_EggDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

template class FixedLimitVector<NintendoSwitch::PokemonSV::EggWatcher>;

namespace NintendoSwitch{
namespace PokemonSV{

namespace {

class EggMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    EggMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Egg-Template.png", Color(100,100,100), Color(255, 255, 255), 50
    ) {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.1;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance() {
        static EggMatcher matcher;
        return matcher;
    }
};

} // end anonymous namespace


EggDetector::EggDetector(BoxCursorLocation side, uint8_t row, uint8_t col, Color color)
: m_color(color) {
    if (side == BoxCursorLocation::PARTY){
        m_box = ImageFloatBox(0.149, 0.1165 * row + 0.235, 0.033, 0.066);
    } else if (side == BoxCursorLocation::SLOTS){
        m_box = ImageFloatBox(0.0656 * col + 0.249, 0.1165 * row + 0.235, 0.033, 0.066);
    } else {
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID BoxCursorLocation for EggDetector");
    }
}

void EggDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool EggDetector::detect(const ImageViewRGB32& frame) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(200, 200, 200), combine_rgb(255, 255, 255)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);

    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 700);
    
    return match_template_by_waterfill(
        extract_box_reference(frame, m_box), 
        EggMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        60,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
}

EggPartyColumnWatcher::EggPartyColumnWatcher(Color color) : VisualInferenceCallback("EggPartyColumnWatcher"), m_egg_watchers(5), m_empty_watchers(5) {
    for(uint8_t i = 0; i < 5; i++){
        m_egg_watchers.emplace_back(
            BoxCursorLocation::PARTY,
            (uint8_t)(i + 1), (uint8_t)0,
            EggWatcher::FinderType::CONSISTENT,
            color
        );
        m_empty_watchers.emplace_back(
            BoxCursorLocation::PARTY,
            (uint8_t)(i + 1), (uint8_t)0,
            BoxEmptySlotWatcher::FinderType::CONSISTENT,
            color
        );
    }
}

void EggPartyColumnWatcher::make_overlays(VideoOverlaySet& items) const{
    for(int i = 0; i < 5; i++){
        m_egg_watchers[i].make_overlays(items);
        m_empty_watchers[i].make_overlays(items);
    }
}

bool EggPartyColumnWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool all_certain = true;
    for(int i = 0; i < 5; i++){
        // Return true if an egg is detected
        const bool egg_certain =  m_egg_watchers[i].process_frame(frame, timestamp);
        // Return true if it is sure that the slot is empty or not
        const bool empty_certain = m_empty_watchers[i].process_frame(frame, timestamp);

        if (!egg_certain || !empty_certain){
            all_certain = false;
        }
    }
    return all_certain;
}

uint8_t EggPartyColumnWatcher::num_eggs_found() const{
    uint8_t num_eggs = 0;
    for(int i = 0; i < 5; i++){
        if (m_egg_watchers[i].consistent_result()){
            num_eggs++;
        }
    }
    return num_eggs;
}

uint8_t EggPartyColumnWatcher::num_non_egg_pokemon_found() const{
    uint8_t num_pokemon = 0;
    for(int i = 0; i < 5; i++){
        if (m_empty_watchers[i].consistent_result() == false){
            num_pokemon++;
        }
    }
    return num_pokemon;
}


}
}
}
