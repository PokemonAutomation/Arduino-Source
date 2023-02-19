/*  Let's Go HP Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_LetsGoHpReader_H
#define PokemonAutomation_PokemonSV_LetsGoHpReader_H

#include <atomic>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{


class LetsGoHpWatcher : public VisualInferenceCallback{
public:
    LetsGoHpWatcher(Color color);

    void clear(){
        m_last_known_value.store(-1, std::memory_order_relaxed);
    }
    double last_known_value() const{
        return m_last_known_value.load(std::memory_order_relaxed);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Color m_color;
    ImageFloatBox m_box;
    std::atomic<double> m_last_known_value;
};




}
}
}
#endif
