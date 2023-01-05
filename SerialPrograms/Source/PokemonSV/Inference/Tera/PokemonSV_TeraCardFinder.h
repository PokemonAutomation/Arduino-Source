/*  Tera Card Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCardFinder_H
#define PokemonAutomation_PokemonSV_TeraCardFinder_H

#include <functional>
#include <deque>
#include <set>
#include <map>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{



std::string partial_tera_card_read_code(
    Logger& logger,
    const ImageViewRGB32& image
);
std::map<Language, std::string> partial_tera_card_read_host(
    Logger& logger,
    const ImageViewRGB32& image,
    const std::set<Language>& languages
);


class PartialTeraCardTracker : public VisualInferenceCallback{
public:
    PartialTeraCardTracker(
        VideoOverlay& overlay, Color color, const ImageFloatBox& box,
        std::function<bool (const ImageViewRGB32& card)> on_card_callback    //  Returns true if "process_frame()" should return true.
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    VideoOverlay& m_overlay;
    Color m_color;
    ImageFloatBox m_box;
    std::function<bool (const ImageViewRGB32& card)> m_on_card_callback;

    SpinLock m_lock;
    std::deque<OverlayBoxScope> m_overlays;
};



}
}
}
#endif
